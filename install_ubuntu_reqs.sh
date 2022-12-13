#!/bin/sh
# installs packages for building on Ubuntu 20.04

set -eu

# required for compiling vlab base components 
base_pkgs='
  g++
  make
  qt5-default
  freeglut3-dev
  bison
  flex
  libreadline6-dev
'

# required for building the documentation
doc_pkgs='
  texlive-latex-base
  texlive-latex-recommended
  texlive-latex-extra
  texlive-fonts-recommended
'

apt-get install $base_pkgs $doc_pkgs 
