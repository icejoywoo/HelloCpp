#include "base64.h"
#include "base64-impl.h"

int encode64(const std::string& input, std::string* output)
{
    return Base64::Encode(input, output) ? 0 : 1;
}

int decode64(const std::string& input, std::string* output)
{
    return Base64::Decode(input, output) ? 0 : 1;
}

int encode64(const char* input, size_t input_size, char* output, size_t output_size)
{
    return Base64::Encode(input, input_size, output, output_size) ? 0 : 1;
}

int decode64(const char* input, size_t input_size, char* output, size_t output_size)
{
    return Base64::Decode(input, input_size, output, output_size) ? 0 : 1;
}

int encode64_length(const char* input, size_t input_size)
{
    return Base64::EncodedLength(input_size);
}

int decode64_length(const char* input, size_t input_size)
{
    return Base64::DecodedLength(input, input_size);
}

