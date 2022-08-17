#include "SelectionScan.h"
#include "Partitioning.h"
#include "IntrinsicApiTest.h"

int main(int argc, const char *argv[])
{
    if (argc != 6) {
        cout << "Wrong arguments num: " << argc << endl;
    }

    int min = atoi(argv[1]);
    int max = atoi(argv[2]);
    int tableSize = atoi(argv[3]);
    int cycle = atoi(argv[4]);
    int partitionNums = atoi(argv[5]);

    runSelectionScan(min, max, tableSize);
    runPartitioning(tableSize, cycle, partitionNums);

    //IntrinsicApiTest simdTest;
    //simdTest.runAllIntrinsicApiTest();
    return 0;
}

