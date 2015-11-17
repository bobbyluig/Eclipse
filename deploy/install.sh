#!/bin/bash

###########################################################################
# Install script for Project Lycanthrope.
# By Lujing Cen.
# Copyright (c) Eclipse Technologies 2015-2016.
# Use only with Debian 8 ("Jessie") ARMv7. Script must be executed as root.
# Requires at least 4GB of free space for installation.
# Designed for MINIBIAN/ODROID Minimal Debian.
# MINIBIAN: https://minibianpi.wordpress.com/
# ODROID Debian: http://forum.odroid.com/viewtopic.php?f=114&t=8084
###########################################################################

#############
# User input.
#############

# Request user for new hostname.
echo "Enter robot hostname: "
read newhost

# Request GitHub password.
echo "Enter GitHub password: "
read password

#####################
# Configure hostname.
#####################

# Get current hostname.
hostn = $(cat /etc/hostname)

# Change hostname. Tengo sed.
sed -i "s/$hostn/$newhost/g" /etc/hosts
sed -i "s/$hostn/$newhost/g" /etc/hostname

###################
# Clone repository.
###################

apt-get -y install git
cd ~
git clone https://bobbyluig:$password@github.com/bobbyluig/Eclipse.git

###############################
# Update and get prerequisites.
###############################ss

# Update and upgrade.
apt-get -y update && apt-get -y upgrade

# Get apt-utils
apt-get -y install apt-utils

# Get required software.
apt-get -y install p7zip-full nano wireless-tools wpasupplicant

#########################
# Python 3 and libraries.
#########################

# Get Python 3.5.
cd ~
apt-get -y install build-essential libssl-dev
wget https://www.python.org/ftp/python/3.5.0/Python-3.5.0.tgz
tar zxvf Python-3.5.0.tgz
cd Python-3.5.0
./configure
make -j4 && make install
cd ..
rm -rf Python-3.5.0
rm -f Python-3.5.0.tgz

# Get Python libraries.
pip3 install numpy pyserial autobahn[accelerate,serialization]
apt-get -y install apt-get install libportaudio0 libportaudio2 libportaudiocpp0 portaudio19-dev
pip3 install pyaudio

####################
# Install OpenCV 3.
####################

# Get required libraries.
apt-get -y install cmake pkg-config
apt-get -y install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
apt-get -y install libatlas-base-dev gfortran

# Get contrib module.
cd ~
git clone https://github.com/Itseez/opencv_contrib.git
cd opencv_contrib
git checkout 3.0.0

# Get version 3.
cd ~
wget https://github.com/Itseez/opencv/archive/3.0.0.zip
7z x 3.0.0.zip
rm -f 3.0.0.zip

# Compile.
cd opencv-3.0.0
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
	-D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=OFF \
	-D INSTALL_PYTHON_EXAMPLES=OFF \
	-D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
	-D BUILD_EXAMPLES=OFF ..
make -j4 && make install
ldconfig

# Cleanup.
cd ~
rm -rf opencv-3.0.0 opencv_contrib

#######################
# Install LLVM + Numba.
#######################

# Get LLVM and dependencies.
cd ~
apt-get -y install libedit-dev libffi-dev
wget http://llvm.org/releases/3.6.2/llvm-3.6.2.src.tar.xz
tar xvfJ llvm-3.6.2.src.tar.xz
rm -f llvm-3.6.2.src.tar.xz

# Get Python 2.7.
apt-get -y install python

# Configure, make, and install.
cd llvm-3.6.2.src
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON -DLLVM_TARGETS_TO_BUILD="ARM" ..
make -j4 && make install

# Cleanup
cd ~
rm -rf llvm-3.6.2.src

# Get llvmlite. Remove static linking.
git clone https://github.com/numba/llvmlite.git
cd llvmlite
# sed -i "s/-static-libstdc++ //g" ffi/Makefile.linux
python3 setup.py build
python3 setup.py install

# Update libraries.
ldconfig

# Get Numba.
pip3 install numba

