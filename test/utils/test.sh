#!/bin/sh

WUK_INSTALL_PREFIX=$1

cmake --build $WUK_INSTALL_PREFIX/build -t test
