import ctypes

lib = ctypes.cdll.LoadLibrary("libbase64.so")

# declare method arg types (optional)
lib.encode64_length.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_size_t)
lib.encode64.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_size_t, ctypes.POINTER(ctypes.c_char), ctypes.c_size_t)
lib.decode64_length.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_size_t)
lib.encode64.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_size_t, ctypes.POINTER(ctypes.c_char), ctypes.c_size_t)

def encode64(input):
    input_size = len(input)

    output_size = lib.encode64_length(input, input_size)
    out = "0" * output_size
    if lib.encode64(input, input_size, ctypes.c_char_p(out), output_size) == 0:
        return out
    else:
        return None

def decode64(input):
    input_size = len(input)

    output_size = lib.decode64_length(input, input_size)
    out = "0" * output_size
    if lib.decode64(input, input_size, ctypes.c_char_p(out), output_size) == 0:
        return out
    else:
        return None

if __name__ == '__main__':
    print(encode64("hello, world"))
    print(decode64(encode64("hello, world")))