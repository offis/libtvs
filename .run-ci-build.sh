#!/usr/bin/env sh

rm -rf objdir
meson -Dwith_systemc=false objdir
cd objdir
ninja test
