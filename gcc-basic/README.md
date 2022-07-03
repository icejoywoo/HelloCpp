# gcc/g++

mac上gcc使用的就是clang。

# makefile

就是gcc编译命令的拼接，有一些默认规则，并且可以自定义依赖关系

# cmake

make VERBOSE=1
可以用来查看编译的命令

cmake -DCMAKE_FIND_DEBUG_MODE=1
可以查看 find_xx 的具体查找路径，有很多 debug 日志

# 其他

查看动态库
* mac: otool -L <executable>
* linux: ldd <executable>

这些东西的含义？
-fPIC

rpath

动态库的查找逻辑

readelf/ patchelf 修改 binary？

RAII 特性

学习 google c++ 编程规范：https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents/
