#!/bin/sh 
# installs packages for building on Fedora 22

set -eu

# required for compiling vlab base components 
base_pkgs='
  gcc-c++
  make
  qt-devel
  flex
  bison
  byacc
  readline-devel
  libpng-devel
  freeglut-devel
'

# required for building the documentation
doc_pkgs='
  texlive
  texlive-adjustbox
  texlive-xtab
  texlive-multirow
  texlive-sectsty
  texlive-tocloft 
'

# required for running vlab
runtime_pkgs='xterm'

dnf install -y $base_pkgs $doc_pkgs $runtime_pkgs
