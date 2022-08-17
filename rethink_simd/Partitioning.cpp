#include "Partitioning.h"
#include <cstring>

int* _mm512_radixHistogram(TableSchema *inputTable, int partitionNum)
{
#define W 16 // vector lane size of 32-bit key in AVX512
#ifdef __AVX2__
    cout << endl << "Begin to do _mm512_radixHistogram, partitions num : " << partitionNum << endl;

    // 只有4个分区
    int partitions = partitionNum;
    // 每个lane都需要一个histogram来表示该位置的分区信息；512中每个vector有16个lane；
    int *histPartial = (int*)malloc(partitions * W * sizeof(int)); //表示p行w列，p为分区数，w为vector lane大小；
    int *hist = (int*)malloc(partitions * sizeof(int)); //记录每个p的rows count；输出的结果；
    memset(histPartial, 0, partitions * W * sizeof(int));

    // 初始化向量
    __m512i mask_shift_left = _mm512_set1_epi32(30); // 因为分区数是4，所以左移30bit，只取最低两位进行radix partition
    __m512i mask_shift_right = _mm512_set1_epi32(30);
    __m512i mask_16 = _mm512_set1_epi32(16);
    __m512i mask_lanes = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,0);
    __m512i mask_1 = _mm512_set1_epi32(1);

    __m512i count;
    for (int i = 0; i < inputTable->tableSize; i += 16) {
        /* load keys and compute the radix */
        __m512i key = _mm512_load_epi32(&inputTable->key[i]);
        printSimdVector(key, "load key");
        __m512i part = _mm512_sllv_epi32(key, mask_shift_left); // 左移30位，把多余的bits移除；
        part = _mm512_srlv_epi32(part, mask_shift_right); // 计算出的part可以理解为分区编号
        printSimdVector(part, "radix part");

        /* compute locations in the replicated histograms */
        // part = _mm512_fmadd_epi32(part, mask_16, mask_lanes); // part对应的index上 = part x mask_16 + mask_lanes

        /*
         * 因为_mm512_fmadd_epi32不支持，所以替换为两个指令；
         * _mm512_mullo_epi32仅保存相乘结果的低32bit，因为分区比较小，32位足够了，所以这个就用lo就ok
         */
        part = _mm512_mullo_epi32(part, mask_16); // 每个分区间的间隔是vector_lane大小，512算法中为16，乘以16后得到分区idx_p；
        printSimdVector(part, "part x mask_16");
        part = _mm512_add_epi32(part, mask_lanes); // 加上每个slot在vector中的idx_v后，计算出来的part就是histPartial的一维index，与hist[idx_p][idx_v]同含义；
        printSimdVector(part, "part + mask_lanes");

        /* increment the partial histograms：更新各location上的值 */
        count = _mm512_i32gather_epi32(part, histPartial, 4); // 把part对应location上的值取出来
        printSimdVector(count, "gather count  ");
        count = _mm512_add_epi32(count, mask_1); // location上的值都增加1；
        printSimdVector(count, "count + mask_1");
        _mm512_i32scatter_epi32(histPartial, part, count, 4); // 加完后赋值到原location上

        // print partial histogram
        cout << "Loop i=" << i << ". After scatter print partial histogram:" << endl;
        for (int p = 0, idx = 0; p != partitions; ++p) {
            for (int w = 0; w != W; ++w,idx++) {
                cout << histPartial[idx] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    /* merge partial histograms */
    for (int p = 0; p != partitions; ++p) {
        count = _mm512_load_epi32(&histPartial[p << 4]); // 按行计算count
        hist[p] = _mm512_reduce_add_epi32(count); // 计算count 512bits中的每个32bit的和
    }

    // print histogram for debug
    cout << "print histogram:" << endl;
    for (int p = 0; p != partitions; ++p) {
        cout << "partition: " << p << ", rows count: " << hist[p] << endl;
    }

    return hist;
#endif
    cout << "AVX not support!" << endl;
}

/**
 * 可以看到解冲突时并没有用到key值，这是因为引入的mask_reverse不仅可以翻转，还能作为unique值用来判断是否冲突
 * mask_reverse 两个作用，一个是用来保证stable；一个是作为unique值用来检测冲突
 * 计算过程：
 * key      ：0 1 2 3 4 0 1 2
 * radixPart：0 1 2 3 0 0 1 2
 * reverse  ：7 6 5 4 3 2 1 0
 * radixPart_reverse: 2 1 0 0 3 2 1 0
 * gather offsets   ：5 6 7 7 4 5 6 7
 * @param radixPart
 * @param baseOffsets
 * @return
 */
__m512i _mm512_serialize_conflicts(__m512i radixPart, int* baseOffsets)
{
#ifdef __AVX2__
    cout << "Begin to do _mm512_serialize_conflicts(__m512i radixPart, int* baseOffsets) " << endl;

    // 初始化向量
    // 注意这个512的set是从最高位开始的，转成int array的话mask_reverse[0] = 15
    __m512i mask_reverse = _mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    printSimdVector(mask_reverse, "  mask_reverse  ");
    __m512i mask_1 = _mm512_set1_epi32(1);

    // 这步主要是为了shuffling的结果是stable的，不打乱input的顺序
    radixPart = _mm512_permutevar_epi32(mask_reverse, radixPart); // 这里注意入参顺序，将radixPart[i] 按照 idx=mask_reverse[i] 重新排列到 radixPart[idx]位置；
    printSimdVector(radixPart, "  radixPart after reverse ");
    __m512i res = _mm512_xor_epi32(res, res); // res为全0的向量；
    __mmask16 m = _mm512_kxnor(m, m); // m为全1的mask

    do {
        /* scatter unique values per lane */ //mask_reverse作为unique的值，用来某个radixPart是否被覆盖写
        _mm512_mask_i32scatter_epi32(baseOffsets, m, radixPart, mask_reverse, 4); //radixPart是scatter的index
        /* gather back values */
        __m512i mask_reverse_back;
        mask_reverse_back = _mm512_mask_i32gather_epi32(mask_reverse_back, m, radixPart, baseOffsets, 4);
        /* detect conflicting lanes */
        m = _mm512_mask_cmpneq_epi32_mask(m, mask_reverse_back, mask_reverse);
        /* increment offsets */
        res = _mm512_mask_add_epi32(res, m, res, mask_1);
    }
    while (!_mm512_kortestz(m, m)); // m全0后表示没有冲突了

    /* reverse result back to original order */
    return _mm512_permutevar_epi32(mask_reverse, res);
#endif
    cout << "AVX not support!" << endl;
}

void _mm512_shufflingByRadixPartitioning(TableSchema *inputTable, int partitionNum)
{
#ifdef __AVX2__
    cout << endl << "Begin to do _mm512_shufflingByRadixPartitioning, partitions num : " << partitionNum << endl;
    // computing partition offsets from radix histogram
    int *histogram = _mm512_radixHistogram(inputTable, partitionNum);
    int *partitionBaseOffs = (int*)malloc(partitionNum * sizeof(int));
    int partitionRows = 0;
    for (int i = 1; i < partitionNum; i++) {
        partitionRows += histogram[i - 1];
        partitionBaseOffs[i] = partitionRows;
    }
    partitionRows += histogram[partitionNum];

    // output buffer是一个确定大小的连续内存
    int *outputKeyBuffer = (int*)malloc(partitionRows * sizeof(int));
    int *outputPayloadBuffer = (int*)malloc(partitionRows * sizeof(int));

    // 初始化向量
    __m512i mask_shift_left = _mm512_set1_epi32(30); // 因为分区数是4，所以左移30bit，只取最低两位进行radix partition
    __m512i mask_shift_right = _mm512_set1_epi32(30);
    __m512i mask_1 = _mm512_set1_epi32(1);

    cout << endl << "Begin to do shuffling" << endl;
    for (int i = 0; i < inputTable->tableSize; i += 16) {
        printArray(partitionBaseOffs, "partitionBaseOffs ", 4);

        /* load keys and payloads */
        __m512i key = _mm512_load_epi32(&inputTable->key[i]);
        __m512i payload = _mm512_load_epi32(&inputTable->payload[i]);
        printSimdVector(key, "key  ");

        /* compute partition function (radix) */
        __m512i partIdx = _mm512_sllv_epi32(key, mask_shift_left);
        partIdx = _mm512_srlv_epi32(partIdx, mask_shift_right);
        printSimdVector(partIdx, "partIdx  ");

        /* load current partition offsets */
        __m512i initOff = _mm512_i32gather_epi32(partIdx, partitionBaseOffs, 4); //part中每个lane的值不可能超过partitionNum
        printSimdVector(initOff, "initOff  ");

        /* detect conflicts (pollutes offsets array) */
        __m512i ser_off = _mm512_serialize_conflicts(partIdx, partitionBaseOffs);
        printSimdVector(ser_off, "ser_off  ");

        /* scatter updated offsets (fixes offsets array) */
        __m512i off_out = _mm512_add_epi32(initOff, ser_off); // off_out是本次要输出的offset，此处不需要+1
        __m512i off_back = _mm512_add_epi32(off_out, mask_1); // off_back值指下一次的baseOffset
        printSimdVector(off_back, "off_back  ");
        _mm512_i32scatter_epi32(partitionBaseOffs, partIdx, off_back, 4); // 通过scatter更新下一次的baseOffset

        // scatter key to output buffer by off_back
        _mm512_i32scatter_epi32(outputKeyBuffer, off_out, key, 4); // 根据本次的off_out（idx）通过scatter将key中的值依次输出到outputBuffer对应的idx位置；
        cout << endl;
    }
    printResult(outputKeyBuffer, nullptr, inputTable->tableSize);
    return;
#endif
    cout << "AVX not support!" << endl;
}

/**
 *
 * @param argc
 * tableSize \ data cycle \ partitions;
 * @param argv
 * @return
 */
int runPartitioning(int tableSize, int cycle, int partitionNums)
{
    TableSchema *inputTable = (TableSchema *)malloc(sizeof(TableSchema));
    initTableInCycle(inputTable, tableSize, cycle);
    printInputTable(inputTable->key, inputTable->payload, inputTable->tableSize);

    // test radix histogram
    //int *hist = _mm512_radixHistogram(inputTable, partitionNums);
    _mm512_shufflingByRadixPartitioning(inputTable, partitionNums);

    return 0;
}
