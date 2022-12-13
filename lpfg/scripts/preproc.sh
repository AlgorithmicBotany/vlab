#!/bin/bash

set -eu -o pipefail

platform_options=''
cc=''
if [[ $OSTYPE == darwin* ]]; then
  # this next line has been or will be modified to
  # specify the actual architecture to build for
    platform_options='-ARCH'
    cc='clang++ -std=gnu++11 -stdlib=libc++'
else
    cc='g++ -std=gnu++11'
fi

$cc $platform_options -x c++ `cat clopt` -I ${LPFGRESOURCES}/include -E -DLINUX $1 > $2
