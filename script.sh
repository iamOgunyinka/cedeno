#!/bin/bash

wget  https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz -O ~/Downloads/boost_181.tar.gz
mkdir ~/Downloads/boost_dir
tar -xzvf ~/Downloads/boost_181.tar.gz -C ~/Downloads/boost_dir
export BOOST_ROOT="~/Downloads/boost_dir/boost_1_81_0/"
sudo apt-get install python3-dev git cmake python3-pip libssl-dev unixodbc-dev
pip3 install pytest
git clone https://github.com/pybind/pybind11 ~/Downloads/pybind11
thisdir=`pwd`
cd ~/Downloads/pybind11
mkdir build && cd $_
cmake ..
make check -j 4
sudo make install -j 4
cd $thisdir
git submodule update --init --recursive
cmake .
make -j 4
