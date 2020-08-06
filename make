#!/usr/bin/env bash

cd build/
make -j$(nproc)
cd -