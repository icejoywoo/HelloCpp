#ifndef RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_TABLESCHEMA_H
#define RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_TABLESCHEMA_H

#include <iostream>
#include <vector>
#include <immintrin.h>
using namespace std;

typedef struct TableSchema
{
    int tableSize;
    int *key;
    int *payload;
} TableSchema;

void initTableInSequence(TableSchema *table, int tableSize);

void initTableInCycle(TableSchema *table, int tableSize, int cycle);

/**
 * print for debug
 */
void printInputTable(int *outKey, int *outPayload, int resultRows);

void printResult(int *outKey, int *outPayload, int resultRows);

#ifdef __AVX2__
void printSimdVector(__m512i vector, const char* vectorName);
void _mm256_printSimdVector(__m256i vector, const char* vectorName);
void _mm128_printSimdVector(__m128i vector, const char* vectorName);
#endif

void printArray(int* array, const char* arrayName, int size);

#endif //RETHINKING_SIMD_VECTORIZATION_FOR_IN_MEMORY_DATABASES_TABLESCHEMA_H

