#!/usr/bin/python3
# encoding: utf-8

import ctypes
import os


def find_dll_path():
    # makefile 写死了 .so 后缀
    if os.path.exists("./libbase64.so"):
        return "./libbase64.so"
    elif os.path.exists("./cmake-build-debug/libbase64.dylib"):
        return "./cmake-build-debug/libbase64.dylib"
    elif os.path.exists("./build/libbase64.dylib"):
        return "./build/libbase64.dylib"
    else:
        raise FileExistsError("base64 dynamic library is non-existed.")


lib = ctypes.cdll.LoadLibrary(find_dll_path())

# declare method arg types (optional)
lib.encode64_length.argtypes = (ctypes.c_char_p, ctypes.c_size_t)
lib.encode64.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t)
lib.decode64_length.argtypes = (ctypes.c_char_p, ctypes.c_size_t)
lib.encode64.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t)


def encode64(input):
    input_size = len(input)

    output_size = lib.encode64_length(input, input_size)
    out = b"0" * output_size
    if lib.encode64(input, input_size, ctypes.c_char_p(out), output_size) == 0:
        return out
    else:
        return None


def decode64(input):
    input_size = len(input)

    output_size = lib.decode64_length(input, input_size)
    out = b"0" * output_size
    if lib.decode64(input, input_size, ctypes.c_char_p(out), output_size) == 0:
        return out
    else:
        return None


if __name__ == '__main__':
    print(encode64(b"hello, world"))
    print(decode64(encode64(b"hello, world")))