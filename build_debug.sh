#!/bin/bash

if [ -e Makefile ]
then
    ./clear_build.sh
fi

cmake -DLOG_DEBUG=ON ./
cmake --build . --parallel 8