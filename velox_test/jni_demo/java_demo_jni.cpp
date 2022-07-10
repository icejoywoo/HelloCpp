#include "JniDemo.h"

#include <iostream>
#include "velox/common/base/BitUtil.h"

#include "velox/common/memory/Memory.h"
#include "velox/functions/Udf.h"
#include "velox/type/Type.h"
#include "velox/vector/BaseVector.h"
#include "velox/connectors/Connector.h"
#include "folly/executors/IOThreadPoolExecutor.h"
#include "velox/connectors/hive/HiveConnector.h"
#include "velox/exec/tests/utils/HiveConnectorTestBase.h"
#include "velox/exec/tests/utils/PlanBuilder.h"
#include "velox/common/file/FileSystems.h"

using namespace facebook::velox;
using namespace facebook::velox::exec::test;

/// This file contains a step-by-step usage example of Velox's expression
/// evaluation engine.
///
/// It shows how to register a simple function and describes all the steps
/// required to create the appropriate query and expression structures, a simple
/// expression tree, an input batch of data, and execute the expression over it.

// First, define a toy function that multiplies the input argument by two.
//
// Check `velox/docs/develop/scalar-functions.rst` for more documentation on how
// to build scalar functions.
template <typename T>
struct TimesTwoFunction {
    FOLLY_ALWAYS_INLINE bool call(int64_t& out, const int64_t& a) {
        out = a * 2;
        return true; // True if result is not null.
    }
};

static int32_t
simpleCountBits(const uint64_t* bits, int32_t begin, int32_t end) {
    int32_t count = 0;
    for (int32_t i = begin; i < end; ++i) {
        count += facebook::velox::bits::isBitSet(bits, i);
    }
    return count;
}

inline int32_t countBits(const uint64_t* bits, int32_t begin, int32_t end) {
    int32_t count = 0;
    facebook::velox::bits::forEachWord(
            begin,
            end,
            [&count, bits](int32_t idx, uint64_t mask) {
                count += __builtin_popcountll(bits[idx] & mask);
            },
            [&count, bits](int32_t idx) {
                count += __builtin_popcountll(bits[idx]);
            });
    return count;
}

/*
 * Class:     JniDemo
 * Method:    runExpressionEval
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JniDemo_runExpressionEval
  (JNIEnv *env, jobject obj) {
    // copy from velox/common/base/tests/BitUtilTest.cpp, TEST_F(BitUtilTest, countBits)
    uint64_t data[] = {
            0xff00ff00ffff00ff,
            0x0f0f0f0f0f0f0f0f,
            0xfedcba9876543210,
            0xf0f0f0f0f0f0f0f0,
            0x0123456789abcdef};
    // We drop one high bit and one low bit on every round.
    int32_t totalBits = sizeof(data) * 8;
    for (int32_t i = 0; i < 10 + (totalBits / 2); ++i) {
        std::cout << countBits(data, i, totalBits - i) << ", "
                << simpleCountBits(data, i, totalBits - i) << std::endl;
    }

    // copy from velox/examples/ExpressionEval.cpp

    // Register the function defined above. The first template parameter is the
    // class that implements the `call()` function (or one of its variations), the
    // second template parameter is the function return type, followed by the list
    // of function input parameters.
    //
    // This function takes as an argument a list of aliases for the function being
    // registered.
    registerFunction<TimesTwoFunction, int64_t, int64_t>({"times_two"});

    // First of all, executing an expression in Velox will require us to create a
    // query context, a memory pool, and an execution context.
    //
    // QueryCtx holds the metadata and configuration associated with a
    // particular query. This is shared between all threads of execution
    // for the same query (one object per query).
    auto queryCtx = core::QueryCtx::createForTest();

    // ExecCtx holds structures associated with a single thread of execution
    // (one per thread). Each thread of execution requires a scoped memory pool,
    // which is where allocations from this thread will be made. When required, a
    // pointer to this pool can be obtained using execCtx.pool().
    //
    // Optionally, one can control the per-thread memory cap by passing it as an
    // argument to getDefaultScopedMemoryPool() - no limit by default.
    auto pool = memory::getDefaultScopedMemoryPool();
    core::ExecCtx execCtx{pool.get(), queryCtx.get()};

    // Next, let's create an expression tree to be executed in this example. On a
    // high-level, our expression tree will have the following structure:
    //
    // -----------------------------
    // | CallTypedExpr (times_two) |  => root
    // -----------------------------
    //            /\
  //            ||
    // ---------------------------------
    // | FieldAccessTypedExpr (my_col) |
    // ---------------------------------
    //
    // Let's first define a type for the input dataset used in this example. In
    // this case, a single input column called "my_col", typed as bigint:
    auto inputRowType = ROW({{"my_col", BIGINT()}});

    // FieldAccessTypedExpr let us choose a particular field/column from the input
    // dataset(s). The first parameter defines the return type of this field, the
    // second is the field name. In this case we're interested in the "my_col"
    // field:
    auto fieldAccessExprNode =
            std::make_shared<core::FieldAccessTypedExpr>(BIGINT(), "my_col");

    // CallTypedExpr will be the root of our expression tree, and defines a
    // function call. The first parameter is the expected return type (bigint in
    // this case), the second is the list of parameter the function takes, and the
    // third is the function name.
    //
    // This will be the root of our expression tree. In a realistic use case, this
    // would be automatically and recursively generated based on some input IDL
    // (or by a SQL string parser).
    auto exprTree = std::make_shared<core::CallTypedExpr>(
            BIGINT(),
            std::vector<core::TypedExprPtr>{fieldAccessExprNode},
            "times_two");

    // Lastly, ExprSet contains the main expression evaluation logic. It takes a
    // vector of expression trees (if there are multiple expressions to be
    // evaluated). ExprSet will output one column per input exprTree. It also
    // takes the execution context associated with the current thread of
    // execution.
    exec::ExprSet exprSet({exprTree}, &execCtx);

    // Generate input batch.
    //
    // The next step is to generate an input batch of data to exercise the
    // expression evaluation code. Expressions are always evaluated using
    // RowVectors as input, which are named collections of vectors.
    //
    // Let's first create a single flat vector to represent the input
    // "my_col" bigint column, and manually fill some data in it:
    const size_t vectorSize = 10;
    auto flatVector = std::dynamic_pointer_cast<FlatVector<int64_t>>(
            BaseVector::create(BIGINT(), vectorSize, execCtx.pool()));
    auto rawValues = flatVector->mutableRawValues();
    std::iota(rawValues, rawValues + vectorSize, 0); // 0, 1, 2, 3, ...

    // Then, let's wrap the generated flatVector in a RowVector:
    auto rowVector = std::make_shared<RowVector>(
            execCtx.pool(), // pool where allocations will be made.
            inputRowType, // input row type (defined above).
            BufferPtr(nullptr), // no nulls for this example.
            vectorSize, // length of the vectors.
            std::vector<VectorPtr>{flatVector}); // the input vector data.

    // Now we move to the actual execution.
    //
    // We first create a vector of VectorPtrs to hold the expression results.
    // (ExprSet outputs one vector per input expression - in this case, 1). The
    // output vector will be allocated internally by ExprSet, so we just need to
    // have a single null VectorPtr in this std::vector.
    std::vector<VectorPtr> result{nullptr};

    // Next, we create an input selectivity vector that controls the visibility
    // of records from the input RowVector. In this case we don't want to filter
    // out any rows, so just create a selectivity vector with all bits set.
    SelectivityVector rows{vectorSize};

    // Before execution we need to create one last structure - EvalCtx - which
    // holds context about the expression evaluation of this particular batch.
    // ExprSets can be reused by the same expression over multiple batches, but we
    // need one EvalCtx per RowVector.
    exec::EvalCtx evalCtx(&execCtx, &exprSet, rowVector.get());

    // Voila! Here we do the actual evaluation. When this function returns, the
    // output vectors will be available in the results vector. Note that ExprSet's
    // logic is synchronous and single threaded.
    exprSet.eval(rows, &evalCtx, &result);

    // Print the output vector, just for fun:
    const auto& outputVector = result.front();
    for (size_t i = 0; i < outputVector->size(); ++i) {
        LOG(INFO) << outputVector->toString(i);
    }

    // Lastly, remember that all allocations are associated with the scoped pool
    // created in the beginning, and moved to ExecCtx. Once ExecCtx dies, it
    // destructs the pool which will deallocate all memory associated with it, so
    // be mindful about the object lifetime!
    //
    // (in this example this is safe since ExecCtx will be destructed last).
}

void writeToFile(
        const std::string& filePath,
        const std::vector<RowVectorPtr>& vectors,
        std::unique_ptr<memory::MemoryPool>& pool_,
        std::shared_ptr<dwrf::Config> config = std::make_shared<facebook::velox::dwrf::Config>()) {
    static const auto kWriter = "HiveConnectorTestBase.Writer";

    facebook::velox::dwrf::WriterOptions options;
    options.config = config;
    options.schema = vectors[0]->type();
    auto sink =
            std::make_unique<facebook::velox::dwio::common::FileSink>(filePath);
    facebook::velox::dwrf::Writer writer{
            options,
            std::move(sink),
            pool_->addChild(kWriter, std::numeric_limits<int64_t>::max())};

    for (size_t i = 0; i < vectors.size(); ++i) {
        writer.write(vectors[i]);
    }
    writer.close();
}

/*
 * Class:     JniDemo
 * Method:    runLimitTest
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JniDemo_runLimitTest
        (JNIEnv * env, jobject obj) {
    filesystems::registerLocalFileSystem();
    std::unique_ptr<folly::IOThreadPoolExecutor> executor_ = std::make_unique<folly::IOThreadPoolExecutor>(3);
    auto hiveConnector = connector::getConnectorFactory(
            connector::hive::HiveConnectorFactory::kHiveConnectorName)
            ->newConnector("test-hive", nullptr, executor_.get());
    connector::registerConnector(hiveConnector);
    dwrf::registerDwrfReaderFactory();

    std::unique_ptr<memory::MemoryPool> pool_{memory::getDefaultScopedMemoryPool()};
    test::VectorMaker vectorMaker_{pool_.get()};
    auto data = vectorMaker_.rowVector({vectorMaker_.flatVector<int32_t>(1'000, [](auto row) { return row; })});

    auto file = TempFilePath::create();
    writeToFile(file->path, {data}, pool_);

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
            scanNodeId, exec::Split(HiveConnectorTestBase::makeHiveConnectorSplit(file->path)));

    int32_t numRead = 0;
    while (cursor.moveNext()) {
        auto vector = cursor.current();
        numRead += vector->size();
    }

    printf("%d\n", numRead);

    printf("%d\n", waitForTaskCompletion(cursor.task().get(), 10'000'000));

    if (executor_) {
        executor_->join();
    }
    dwrf::unregisterDwrfReaderFactory();
    connector::unregisterConnector(kHiveConnectorId);
}
