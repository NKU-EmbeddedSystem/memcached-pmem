#!/bin/bash

g++ client_get.cpp -o client_get -ggdb -O3 -lmemcached
./client_get -s localhost -p 11211

