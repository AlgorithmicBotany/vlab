#!/bin/sh
# installs packages for building on Ubuntu 22.04.5 LTS

set -eu

# required for compiling vlab base components 
base_pkgs='
  g++
  make
  qt6-base-dev
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
