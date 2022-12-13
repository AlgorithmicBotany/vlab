clang++ -std=gnu++11 -stdlib=libc++ -x c++ `cat clopt` -I ${LPFGRESOURCES}/include -E -DLINUX $1 > $2
