#include <iostream>
#include <string>
#include "base64.h"
#include "base64-impl.h"

int main(int argc, char const *argv[])
{
    std::string output;
    Base64::Encode("Hello, World!", &output);
    // if (encode64("Hello, World!", &output) != 0) {
    //     std::cerr << "failed to encode64" << std::endl;
    //     return EXIT_FAILURE;
    // }
    std::cout << output << std::endl;
    return EXIT_SUCCESS;
}
