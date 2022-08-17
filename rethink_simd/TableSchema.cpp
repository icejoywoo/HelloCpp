#include "TableSchema.h"

void initTableInSequence(TableSchema *table, int tableSize)
{
    cout << "Begin to init table data:" << endl;
    std::vector<int>* key = new std::vector<int>; // allocate on heap
    std::vector<int>* payload = new std::vector<int>;
    for (int i = 0; i < tableSize; i++) {
        key->push_back(i);
        payload->push_back(i + 1);
    }

    // TODO: here should do memcpy
    table->tableSize = tableSize;
    table->key = key->data();
    table->payload = payload->data();

    cout << "TableSize: " << tableSize << ", key size: " << key->size() << ", payload size: " << payload->size() << endl;
}

void initTableInCycle(TableSchema *table, int tableSize, int cycle)
{
    cout << "Begin to init table data in cycle: " << cycle << endl;
    table->tableSize = tableSize;
    table->key = (int*)malloc(tableSize * sizeof(int));
    table->payload = (int*)malloc(tableSize * sizeof(int));

    for (int i = 0; i < tableSize; i++) {
        int val = i % cycle;
        table->key[i] = val;
        table->payload[i] = (val + 1);
    }
    cout << "TableSize: " << tableSize << ", key size: " << tableSize << ", payload size: " << tableSize << endl;
}

void printInputTable(int *outKey, int *outPayload, int resultRows)
{
    cout << "Print InputTable:" << endl;

    cout << "inputKeys: ";
    for (int i = 0; i < resultRows; i++) {
        cout << outKey[i] << " ";
    }
    cout << endl;

    cout << "inputPayloads: ";
    for (int i = 0; i < resultRows; i++) {
        cout << outPayload[i] << " ";
    }
    cout << endl;
}

void printResult(int *outKey, int *outPayload, int resultRows) {
    cout << "Print results:" << endl;

    if (outKey != nullptr) {
        cout << "outKeys: ";
        for (int i = 0; i < resultRows; i++) {
            cout << outKey[i] << " ";
        }
        cout << endl;
    }

    if (outPayload != nullptr) {
        cout << "outPayloads: ";
        for (int i = 0; i < resultRows; i++) {
            cout << outPayload[i] << " ";
        }
        cout << endl;
    }
}

#ifdef __AVX2__
void printSimdVector(__m512i vector, const char* vectorName)
{
    int *tmp = (int *)malloc(sizeof(int) * 16);
    _mm512_store_epi32(&tmp[0], vector);
    cout << vectorName <<": ";
    for (int w = 0; w != 16; ++w) {
        cout << tmp[w] << " ";
    }
    cout << endl;
}

void _mm256_printSimdVector(__m256i vector, const char* vectorName)
{
    const int LANES = 8;
    int tmp[LANES];
    _mm256_store_epi32(&tmp[0], vector);
    cout << vectorName <<": ";
    for (int w = 0; w != LANES; ++w) {
        cout << tmp[w] << " ";
    }
    cout << endl;
}

void _mm128_printSimdVector(__m128i vector, const char* vectorName)
{
    const int LANES = 4;
    int tmp[LANES];
    _mm_store_epi32(&tmp[0], vector);
    cout << vectorName <<": ";
    for (int w = 0; w != LANES; ++w) {
        cout << tmp[w] << " ";
    }
    cout << endl;
}
#endif

void printArray(int* array, const char* arrayName, int size)
{
    cout << arrayName <<": ";
    for (int w = 0; w != size; ++w) {
        cout << array[w] << " ";
    }
    cout << endl;
}

