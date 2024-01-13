#!/bin/bash

g++ client_set.cpp -o client_set -ggdb -O3 -lmemcached
./client_set -s localhost -p 11211

