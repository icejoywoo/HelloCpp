/**
 * sysctl -a | grep machdep.cpu.features
 *
 * g++ -mavx2 -mavx512f -std=c++11 SelectScan.cpp -o SelectScan
 *
 * software.intel.com/sites/landingpage/IntrinsicsGuide/
*/

#include "SelectionScan.h"

#include <bitset>

int scalarBranch(TableSchema *table, int min, int max)
{
    cout << "Begin to do ScalarBranch" << endl;

    std::vector<int> outKey; // allocate on stack
    std::vector<int> outPayload;
    for (int i = 0; i < table->tableSize; i++) {
        int key = table->key[i];
        if ( key >= min && key <= max) {
            outPayload.push_back(table->payload[i]);
            outKey.push_back(key);
        }
    }

    printResult(outKey.data(), outPayload.data(), outKey.size());
    return outKey.size();
}

int scalarBranchLess(TableSchema *table, int min, int max)
{
    cout << "Begin to do ScalarBranchLess" << endl;
    std::vector<int> outKey;
    std::vector<int> outPayload;
    int j = 0;
    for (int i = 0; i < table->tableSize; i++) {
        int key = table->key[i];

        outKey.resize(j + 1);
        outPayload.resize(j + 1);
        outPayload[j] = table->payload[i];
        outKey[j] = key;

        int m = ((key >= min) ? 1 : 0) & ((key <= max) ? 1 : 0);
        j = j + m;
    }

    printResult(outKey.data(), outPayload.data(), j);
    return j;
}

// TODO: why??
__m128i* prepare_perm(__m128i* perm)
{
    for (uint16_t i = 0; i < 256; i++) {
        std::bitset<8> bits(i);
        std::vector<uint16_t> on;
        std::vector<uint16_t> off;

        for (uint16_t j = 0; j < 8; j++) {
            if (bits[j]) {
                on.push_back(j);
            } else {
                off.push_back(j);
            }
        }

        uint16_t a[8];
        int j = 0;
        for (std::vector<uint16_t>::iterator it = on.begin(); it != on.end(); ++it, ++j) {
            a[j] = *it;
        }
        for (std::vector<uint16_t>::iterator it = off.begin(); it != off.end(); ++it, ++j) {
            a[j] = *it;
        }
        perm[i] = _mm_set_epi16(a[7], a[6], a[5], a[4], a[3], a[2], a[1], a[0]);
    }
    return perm;
}
/**
 *
 * Begin to do _mm256_vectorSelectiveStoreInDirect
key: 0 1 2 3 4 5 6 7
cmp_lo: -1 -1 0 0 0 0 0 0
cmp_hi: 0 0 0 0 0 0 -1 -1
cmp_or: -1 -1 0 0 0 0 -1 -1
mask_minus_1: -1 -1 -1 -1 -1 -1 -1 -1
cmp_xor: 0 0 -1 -1 -1 -1 0 0
m: 60
perm_comp : 196610 327684 65536 458758
perm : 2 3 4 5 0 1 6 7
cmp after perm : -1 -1 -1 -1 0 0 0 0
cmp after shift: 1 1 1 1 0 0 0 0
rid after perm : 5 4 3 2 7 6 1 0
store into buf : 0 0 0 0 0 0 0 0 // mask store没work why？？
k 4
Print results:
outKeys: 0 0 0 0
outPayloads: 1 1 1 1
_mm256_vectorSelectiveStoreInDirect resultRows:4
 * @param table
 * @param min
 * @param max
 * @return
 */

int _mm256_vectorSelectiveStoreInDirect(TableSchema *table, int min, int max)
{
#ifdef __AVX2__
    cout << "Begin to do _mm256_vectorSelectiveStoreInDirect" << endl;
    int *outKey = (int *) malloc(sizeof(int) * 320); // 为了内存对齐32字节
    int *outPayload = (int *) malloc(sizeof(int) * 320);

    int i, j, k;
    __m256i mask_lower = _mm256_set1_epi32(min); // 使用整型min填充向量mask_lower
    __m256i mask_upper= _mm256_set1_epi32(max); // 使用整型max填充向量mask_lower
    __m256i mask_minus_1 = _mm256_set1_epi32(-1); // 用于异或取值
    __m256i mask_8 = _mm256_set1_epi32(8); // 用于rid每次循环的偏移
    __m256i mask_shift_right = _mm256_set1_epi32(31);
    __m256i mask_1 = _mm256_set1_epi32(1);

    int buf_size = 1024;
    int *rids_buf = (int *) malloc(sizeof(int) * buf_size);
    __m256i rid = _mm256_set_epi32(0, 1, 2, 3, 4, 5, 6, 7);

    // prepare the permutation array
    __m128i perm[256];
    prepare_perm(perm);

    for (i = j = k = 0; i < table->tableSize; i += 8) {
        /* load key columns and evaluate predicates */
        __m256i key = _mm256_load_si256(reinterpret_cast<__m256i*>(&table->key[i]));
        __m256i cmp_lo = _mm256_cmpgt_epi32(mask_lower, key);
        __m256i cmp_hi = _mm256_cmpgt_epi32(key, mask_upper);
        __m256i cmp_or = _mm256_or_si256(cmp_lo, cmp_hi);
        __m256i cmp = _mm256_xor_si256(cmp_or, mask_minus_1);

        if (!_mm256_testz_si256(cmp, cmp)) { // _mm256_testz_si256计算cmp中至少有一bit为1时，返回0；表示有命中的行
            _mm256_printSimdVector(key, "key");
            _mm256_printSimdVector(cmp_lo, "cmp_lo");
            _mm256_printSimdVector(cmp_hi, "cmp_hi");
            _mm256_printSimdVector(cmp_or, "cmp_or");
            _mm256_printSimdVector(mask_minus_1, "mask_minus_1");
            _mm256_printSimdVector(cmp, "cmp_xor");

            /* （1）load permutation mask for selective store */
            int m = _mm256_movemask_ps(_mm256_castsi256_ps(cmp)); // 将符号位(命中)写到m低字节的bits[7:0];
            cout << "m: "<< m << endl;

            __m128i perm_comp = perm[m];
            _mm128_printSimdVector(perm_comp, "perm_comp ");

            __m256i perm = _mm256_cvtepi16_epi32(perm_comp);
            _mm256_printSimdVector(perm, "perm ");

            /* （2）permute and store the input pointers */
            cmp = _mm256_permutevar8x32_epi32(cmp, perm); // cvt_cmp按照perm里的idx重新排列
            _mm256_printSimdVector(cmp, "cmp after perm ");

            cmp = _mm256_srlv_epi32(cmp, mask_shift_right);
            _mm256_printSimdVector(cmp, "cmp after shift");

            __m256i rid_perm = _mm256_permutevar8x32_epi32(rid, perm);
            _mm256_printSimdVector(rid_perm, "rid after perm ");

            // （3）selective store
            _mm256_maskstore_epi32(rids_buf + k, cmp, rid_perm); // mask store没生效？？？
            __m256i buf = _mm256_load_si256(reinterpret_cast<__m256i*>(&rids_buf[k]));
            _mm256_printSimdVector(buf, "buf ");

            // (4) cache rids into buffer
            k += _mm_popcnt_u64(m); // 计算m中为1的bit数
            cout<<"k "<<k<<endl;
            int b;
            __m256i ptr;
            if (k > buf_size - 8) {
                /* flush the buffer */
                for (b = 0; b != buf_size - 8; b += 8) {
                    /* dereference column values and store */
                    ptr = _mm256_load_si256(reinterpret_cast<const __m256i*>(rids_buf + j));
                    key = _mm256_i32gather_epi32(table->key, ptr, 4);
                    __m256i pay = _mm256_i32gather_epi32(table->payload, ptr, 4);
                    _mm256_stream_si256(reinterpret_cast<__m256i*>(outKey + b + j), key);
                    _mm256_stream_si256(reinterpret_cast<__m256i*>(outPayload + b + j), pay);
                }
                /* move extra items to the start of the buffer */
                ptr = _mm256_load_si256(reinterpret_cast<const __m256i*>(rids_buf + b));
                _mm256_store_si256(reinterpret_cast<__m256i*>(rids_buf), ptr);
                j += buf_size - 8;
                k -= buf_size - 8;
            }
        }

        rid = _mm256_add_epi32(rid, mask_8);
    }

    // in order to output the left keys in buffer
    if (k > 0) {
        __m256i ptr;
        /* flush the buffer */
        for (int i = 0; i < k; i += 8) {
            /* dereference column values and store */
            ptr = _mm256_load_si256(reinterpret_cast<const __m256i*>(rids_buf + i));
            __m256i key = _mm256_i32gather_epi32(table->key, ptr, 4);
            __m256i pay = _mm256_i32gather_epi32(table->payload, ptr, 4);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(outKey + i), key);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(outPayload + i), pay);
        }
    }

    printResult(outKey, outPayload, k);
    return k;
#endif
    cout << "AVX not support!" << endl;
    return 0;
}

void _mm512_mask_packstore_epi32(int32_t *p, // pointer
                                 __mmask16 m, // mask
                                 __m512i v) // vector
{
    //_mm512_mask_packstorelo_epi32(&p[0], m, v);
    //_mm512_mask_packstorehi_epi32(&p[16], m, v);
}

/**
 * requires target feature 'avx512f'
 * @param table
 * @param min
 * @param max
 * @return
 */
int _mm512_vectorSelectiveStoreInDirect(TableSchema *table, int min, int max)
{
#ifdef __AVX2__
    cout << "Begin to do _mm512_vectorSelectiveStoreInDirect" << endl;
    int *outKey = (int *) malloc(sizeof(int) * 320); // 为了内存对齐32/64字节
    int *outPayload = (int *) malloc(sizeof(int) * 320);

    int i, j, k;
    int resultRows = 0; // 为了最后输出buffer中所有值
    __m512i mask_lower = _mm512_set1_epi32(min); // 使用整型min填充向量mask_lower
    __m512i mask_upper= _mm512_set1_epi32(max); // 使用整型min填充向量mask_lower
    __m512i mask_16 = _mm512_set1_epi32(16); // 用于rid每次循环的偏移

    int buf_size = 1024;
    int *rids_buf = (int *) malloc(sizeof(int) * buf_size);
    __m512i rid = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,0);

    for (i = j = k = 0; i < table->tableSize; i += 16) {
        /* load key column and evaluate predicates */
        __m512i key = _mm512_load_epi32(&table->key[i]);
        __mmask16 m = _mm512_cmpge_epi32_mask(key, mask_lower);
        m = _mm512_mask_cmple_epi32_mask(m, key, mask_upper);

        if (!_mm512_kortestz(m, m)) { // jkzd
            /* selectively store qualifying tuple indexes */
            // _mm512_mask_packstore_epi32(&rids_buf[k], m, rid); // 这个指令不支持; mask_packstore是指从rids_buffer入参位置开始依次写入mask命中的值；
            // _mm512_mask_store_epi32(&rids_buf[k], m, rid); // 注意，这个指令与packstore不等价，结果是错误的；mask_store根据mask是否命中决定是否store该mask_idx对应的值；如果idx位置命中则赋值：rids_buf[idx] = rid[idx]；

            __m512i tmp = _mm512_setzero_si512();
            tmp = _mm512_mask_compress_epi32(tmp, m, rid);
            _mm512_store_epi32(&rids_buf[k], tmp);

            int hitRows = _mm_popcnt_u64(m); // k += _mm_countbits_64(_mm512_mask2int(m));
            k += hitRows;
            resultRows += hitRows;
        }

        // buffer is full
        if (k > buf_size - 16) {
            /* flush the buffer */
            int b = 0;
            for (; b != buf_size - 16; b += 16) {
                __m512i ptr = _mm512_load_epi32(&rids_buf[b]);
                /* dereference column values and stream */
                __m512 key_f = _mm512_i32gather_ps(ptr, table->key, 4);
                __m512 pay_f = _mm512_i32gather_ps(ptr, table->payload, 4);
                _mm512_store_ps(&outKey[b + j], key_f); //_mm512_storenrngo_ps(&outKey[b + j], key_f);
                _mm512_store_ps(&outPayload[b + j], pay_f); //_mm512_storenrngo_ps(&outPayload[b + j], pay_f);
            }
            /* move extra items to the start of the buffer */
            __m512i ptr = _mm512_load_epi32(&rids_buf[b]);
            _mm512_store_epi32(&rids_buf[0], ptr);
            j += buf_size - 16;
            k -= buf_size - 16;
        }
        rid = _mm512_add_epi32(rid, mask_16);
    }

    // in order to output the last key rows
    if (k > 0) {
        int b = 0;
        for (; b < k; b += 16) {
            __m512i ptr = _mm512_load_epi32(&rids_buf[b]);
            /* dereference column values and stream */
            __m512 key_f = _mm512_i32gather_ps(ptr, table->key, 4);
            __m512 pay_f = _mm512_i32gather_ps(ptr, table->payload, 4);
            _mm512_store_ps(&outKey[b + j], key_f);
            _mm512_store_ps(&outPayload[b + j], pay_f);
        }
        k -= b;
    }

    printResult(outKey, outPayload, resultRows);
    return resultRows;
#endif
    cout << "AVX not support!" << endl;
}

/**
 *
 * @param argc
 * min max tableSize;
 * @param argv
 * @return
 */
int runSelectionScan(int min, int max, int tableSize)
{
    TableSchema *InputTable = (TableSchema *)malloc(sizeof(TableSchema));
    initTableInSequence(InputTable, tableSize);
    printInputTable(InputTable->key, InputTable->payload, InputTable->tableSize);

    cout << "Begin to do selection scan, min:" << min << ", max:" << max << endl << endl;

    int resultRows = 0;
    resultRows = scalarBranch(InputTable, min, max);
    cout << "ScalarBranch resultRows:" << resultRows << endl << endl;

    resultRows = scalarBranchLess(InputTable, min, max);
    cout << "ScalarBranchLess resultRows:" << resultRows << endl << endl;

    resultRows = _mm256_vectorSelectiveStoreInDirect(InputTable, min, max);
    cout << "_mm256_vectorSelectiveStoreInDirect resultRows:" << resultRows << endl << endl;

    resultRows = _mm512_vectorSelectiveStoreInDirect(InputTable, min, max);
    cout << "VectorSelectiveStoreInDirect_512 resultRows:" << resultRows << endl;

    return 0;
}
