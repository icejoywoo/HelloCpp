#pragma once

#include <string>

int encode64(const std::string& input, std::string* output);

int decode64(const std::string& input, std::string* output);

#ifdef __cplusplus
extern "C" {
// extern 的作用，到处的函数名在c++中会改名，c中不会，一个可以的用途是在 python 等语言中直接调用其动态库的代码
#endif /* __cplusplus */

int encode64(const char* input, size_t input_size, char* output, size_t output_size);

int encode64_length(const char* input, size_t input_size);

int decode64(const char* input, size_t input_size, char* output, size_t output_size);

int decode64_length(const char* input, size_t input_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
