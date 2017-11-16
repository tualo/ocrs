#!/bin/bash


clang++  -Wall -std=c++14 test_reg.cpp -o test_reg -lboost_system -lboost_regex -lboost_filesystem -lboost_thread-mt -lboost_chrono -lboost_iostreams -lboost_atomic-mt -lboost_date_time -lpthread
