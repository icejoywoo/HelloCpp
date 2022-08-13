#include "arrow/api.h"
#include "arrow/compute/api.h"
#include "simdjson.h"
#include "arrow/visit_data_inline.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using namespace simdjson;

// Demonstrate registering a user-defined Arrow compute function outside of the Arrow
// source tree

namespace cp = ::arrow::compute;

#define ABORT_ON_FAILURE(expr)                     \
  do {                                             \
    arrow::Status status_ = (expr);                \
    if (!status_.ok()) {                           \
      std::cerr << status_.message() << std::endl; \
      abort();                                     \
    }                                              \
  } while (0);

template <typename TYPE,
        typename = typename std::enable_if<arrow::is_number_type<TYPE>::value |
                                           arrow::is_boolean_type<TYPE>::value |
                                           arrow::is_temporal_type<TYPE>::value>::type>
arrow::Result<std::shared_ptr<arrow::Array>> GetArrayDataSample(
        const std::vector<typename TYPE::c_type>& values) {
    using ArrowBuilderType = typename arrow::TypeTraits<TYPE>::BuilderType;
    ArrowBuilderType builder;
    ARROW_RETURN_NOT_OK(builder.Reserve(values.size()));
    ARROW_RETURN_NOT_OK(builder.AppendValues(values));
    return builder.Finish();
}

const cp::FunctionDoc json_func_doc{
        "User-defined-function usage to extract json id_str from test input",
        "returns x",
        {"x"},
        "UDFOptions"};

arrow::Status JsonExtractFunction(cp::KernelContext* ctx, const cp::ExecSpan& batch,
                                  cp::ExecResult* out) {
    static ondemand::parser parser;

    using offset_type = arrow::StringType::offset_type;
    using ValueDataBuilder = arrow::TypedBufferBuilder<uint8_t>;
    using OffsetBuilder = arrow::TypedBufferBuilder<offset_type>;

    ValueDataBuilder value_data_builder(ctx->memory_pool());
    OffsetBuilder offset_builder(ctx->memory_pool());

    // We already know how many strings we have, so we can use Reserve/UnsafeAppend
    RETURN_NOT_OK(offset_builder.Reserve(batch.length + 1));
    offset_builder.UnsafeAppend(0);  // offsets start at 0

    RETURN_NOT_OK(arrow::VisitArraySpanInline<arrow::StringType>(
            batch[0].array,
            [&](arrow::util::string_view s) {
                ondemand::document doc = parser.iterate(padded_string_view(s.data(), s.size()));
                auto x = doc["id_str"].get_string();
                RETURN_NOT_OK(value_data_builder.Append(
                        reinterpret_cast<const uint8_t *>(x.value().data()),
                        static_cast<int64_t>(x.value().size())));
                offset_builder.UnsafeAppend(
                        static_cast<offset_type>(value_data_builder.length()));
                return arrow::Status::OK();
            },
            [&]() {
                // offset for null value
                offset_builder.UnsafeAppend(
                        static_cast<offset_type>(value_data_builder.length()));
                return arrow::Status::OK();
            }));
    arrow::ArrayData* output = out->array_data().get();
    RETURN_NOT_OK(value_data_builder.Finish(&output->buffers[2]));
    return offset_builder.Finish(&output->buffers[1]);
}

arrow::Status ExecuteJson() {
    const std::string name = "json_extract";
    auto func = std::make_shared<cp::ScalarFunction>(name, cp::Arity::Unary(), json_func_doc);
    cp::ScalarKernel kernel(
            {arrow::utf8()},
            arrow::utf8(), JsonExtractFunction);

    kernel.mem_allocation = cp::MemAllocation::PREALLOCATE;
    kernel.null_handling = cp::NullHandling::INTERSECTION;

    ARROW_RETURN_NOT_OK(func->AddKernel(std::move(kernel)));

    auto registry = cp::GetFunctionRegistry();
    ARROW_RETURN_NOT_OK(registry->AddFunction(std::move(func)));

    std::shared_ptr<arrow::Array> string_array;
    int rows = 100;
    arrow::StringBuilder builder;
    ARROW_RETURN_NOT_OK(builder.Reserve(rows));
    constexpr const char* value = R"({
    "created_at": "Sun Aug 31 00:29:15 +0000 2014",
    "id": 505874924095815681,
    "id_str": "505874924095815681",
    "text": "@aym0566x \n\n名前:前田あゆみ\n第一印象:なんか怖っ！\n今の印象:とりあえずキモい。噛み合わない\n好きなところ:ぶすでキモいとこ😋✨✨\n思い出:んーーー、ありすぎ😊❤️\nLINE交換できる？:あぁ……ごめん✋\nトプ画をみて:照れますがな😘✨\n一言:お前は一生もんのダチ💖"
})";
    for (int i = 0; i < 100; ++i) {
        // add SIMDJSON_PADDING in length
        ARROW_RETURN_NOT_OK(builder.Append(reinterpret_cast<const uint8_t*>(value), strlen(value) + SIMDJSON_PADDING));
    }
    ARROW_RETURN_NOT_OK(builder.Finish(&string_array));
    ARROW_ASSIGN_OR_RAISE(auto res, cp::CallFunction(name, {string_array}));
    auto res_array = res.make_array();
    std::cout << "Result: " << res_array->length() << std::endl;
    std::cout << res_array->ToString() << std::endl;
    return arrow::Status::OK();
}

// refer: arrow/cpp/examples/arrow/udf_example.cc
int main(int argc, char** argv) {
    auto status = ExecuteJson();
    if (!status.ok()) {
        std::cerr << "Error occurred : " << status.message() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
