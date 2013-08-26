#!/bin/sh

tar xf m4-1.4.16.tar.gz
cd m4-1.4.16
./configure --prefix="/devel/sw/" --program-suffix="-1.4.16"
make
make install
cd ..
rm -rf m4-1.4.16

tar xf bison-2.7.tar.gz
cd bison-2.7
patch data/yacc.c ../yacc.c.diff
./configure --prefix="/devel/sw/" --program-suffix="-2.7"
make
make install
cd ..
rm -rf bison-2.7

tar xf flex-2.5.37.tar.bz2
cd flex-2.5.37
./configure --prefix="/devel/sw/" --program-suffix="-2.5.37"
make
make install
cd ..
rm -rf flex-2.5.37

tar xf libelf-0.8.9.tar.gz
cd libelf-0.8.9.tar
./configure 
make
make install
cd ..
rm -rf libelf-0.8.9
