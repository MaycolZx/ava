#!/bin/bash

g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread

# hexdump -e '10/8 "%5.2f " "\n"' R.bin 
