#ifndef RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_INTRINSICAPITEST_H
#define RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_INTRINSICAPITEST_H

#include "TableSchema.h"

class IntrinsicApiTest {
public:
    IntrinsicApiTest();

    virtual ~IntrinsicApiTest() = default;

    /**
     * test permute
     */
     void _m512i_permute_test();

     void _m512i_shuffle_test();

     void runAllIntrinsicApiTest();

private:
    __m512i mask_seq;
    __m512i mask_lower;
    __m512i a;
    __m512i b;
    __m512i idx;
    __mmask16 mask16;
};


#endif //RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_INTRINSICAPITEST_H

