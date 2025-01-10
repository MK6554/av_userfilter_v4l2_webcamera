#!/bin/bash

if [ -e Makefile ]
then
    ./clear_build.sh
fi

cmake .
cmake --build . --parallel 8