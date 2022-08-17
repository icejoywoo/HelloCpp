#include "IntrinsicApiTest.h"

IntrinsicApiTest::IntrinsicApiTest() {
    cout << "Begin to do basic SIMD intrinsic api test " << endl;
    /**
     * init mask vector
     */
    mask_seq = _mm512_set_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 0, 1, 3,2);
    mask_lower = _mm512_set1_epi32(2);

    /**
     * init input vector
     */
    a = _mm512_set_epi32(10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 14, 13, 12, 11);
    b = _mm512_set_epi32(20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 18, 17, 16, 15);
    idx = _mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 4, 0, 2);

    /**
     * create mask16: mask_seq中 <= mask_lower的bit位置赋1
     * mask16: 1, 0, 1, 1, 0, 0....
     */
    mask16 = _mm512_cmple_epi32_mask(mask_seq, mask_lower);
    __m512i mask16_vector = _mm512_movm_epi32(mask16);
    printSimdVector(mask16_vector, "mask16_vector ");
}

void IntrinsicApiTest::_m512i_permute_test() {
    cout << "Permute instructions test: " << endl;

    /************** begin to test ************************/
    /**
     * (1) test permute vector->vector
     */
    __m512i dst = _mm512_set1_epi32(0);
    dst = _mm512_permutevar_epi32(idx, a);
    printSimdVector(dst, "(1)_mm512_permutevar_epi32 dst ");

    /**
     * (2) 512_mask_permutex2var_epi32
     * 三种控制流：
     *  参数mask16决定是否根据idx是否有效；idx第5bit决定排列a还是b的数据；idx的低4位值决定a和b中被排列的位置
     * 三种case：
     *   ① 命中且idx第五bit为0，取a的idx位置数据
     *   ② 未命中，取a的idx位置数据
     *   ③ 命中且idx第五bit为1，取b的idx位置数据
     * 例如：
     * idx为：.....0, 17, 4, 0, 2
     * dst为：.....10, 16, 10, 12, 13
     */
    dst = _mm512_set1_epi32(0);
    dst = _mm512_mask_permutex2var_epi32(a, mask16, idx, b);
    printSimdVector(dst, "(2)_mm512_mask_permutex2var_epi32 dst ");
}

void IntrinsicApiTest::_m512i_shuffle_test() {
    cout << "Shuffle instructions test: " << endl;

    /**
     * （1）_mm512_shuffle_epi32
     * imm8 = 0b00110110;
     * imm8[1:0] 10 = 2  表示取4个int为一组中的第二个int
     * imm8[3:2] 01 = 1
     * imm8[5:4] 11 = 3
     * imm8[6:7] 00 = 0
     * dst : 13 12 14 11 10 10 10 10 10 10 10 10 10 10 10 10
     */
    __m512i dst = _mm512_shuffle_epi32(a, 0b00110110);
    printSimdVector(dst, "(1）_mm512_shuffle_epi32 dst ");

    dst = _mm512_mask_shuffle_epi32(b, mask16, a, 0b00110110);
    printSimdVector(dst, "(1）_mm512_shuffle_epi32 dst ");
}

void IntrinsicApiTest::runAllIntrinsicApiTest() {
    _m512i_permute_test();
    _m512i_shuffle_test();
}
