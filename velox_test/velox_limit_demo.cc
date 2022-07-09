#include "velox/exec/tests/utils/HiveConnectorTestBase.h"
#include "velox/exec/tests/utils/PlanBuilder.h"

using namespace facebook::velox;
using namespace facebook::velox::exec::test;

class LimitTest : public HiveConnectorTestBase {
public:
    void TestBody() {};
    void limitOverLocalExchange() {
        auto data = makeRowVector(
                {makeFlatVector<int32_t>(1'000, [](auto row) { return row; })});

        auto file = TempFilePath::create();
        writeToFile(file->path, {data});

        auto planNodeIdGenerator = std::make_shared<PlanNodeIdGenerator>();
        core::PlanNodeId scanNodeId;

        CursorParameters params;
        params.planNode =
                PlanBuilder(planNodeIdGenerator)
                        .localPartition(
                                {},
                                {PlanBuilder(planNodeIdGenerator)
                                         .tableScan(
                                                 std::dynamic_pointer_cast<const RowType>(data->type()))
                                         .capturePlanNodeId(scanNodeId)
                                         .planNode()})
                        .limit(0, 20, true)
                        .planNode();

        TaskCursor cursor(params);
        cursor.task()->addSplit(
                scanNodeId, exec::Split(makeHiveConnectorSplit(file->path)));

        int32_t numRead = 0;
        while (cursor.moveNext()) {
            auto vector = cursor.current();
            numRead += vector->size();
        }

        waitForTaskCompletion(cursor.task().get());
    }
};

int main() {
    auto test1 = new LimitTest();
    test1->SetUp();
    test1->limitOverLocalExchange();
    test1->TearDown();
    return 0;
}