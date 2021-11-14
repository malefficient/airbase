#!/bin/bash
echo "installing library"
install -m 755 ./libairware.a /usr/local/lib
echo  "installing header files."
mkdir /usr/local/include/airware
rm -f /usr/local/include/airware/*
install -m 755 ./include/* /usr/local/include/airware
