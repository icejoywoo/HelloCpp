#!/bin/bash

set -u
set -e

CURRENT_DIR=$(cd `dirname $0` && pwd)
PROJECT_DIR=${CURRENT_DIR}/../
JAVA_HOME=$(/usr/libexec/java_home -v 1.8)

#${JAVA_HOME}/bin/javac -h ${PROJECT_DIR}/jni JniDemo.java
${JAVA_HOME}/bin/javac -h ${CURRENT_DIR} JniDemo.java

#g++ -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/darwin JniDemo.cc -o JniDemo.o
#g++ -dynamiclib -o libjnidemo.dylib JniDemo.o -lc

java -cp . -Djava.library.path=${PROJECT_DIR}/cmake-build-debug/jni_demo/ JniDemo
