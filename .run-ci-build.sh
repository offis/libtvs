#!/usr/bin/env sh

rm -rf objdir
meson -Dwith_systemc=false -Dcpp_std=c++14 objdir
cd objdir
ninja test
