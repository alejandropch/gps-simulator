#!/bin/bash

gcc -std=gnu17 -Wall -Wextra main.c utils/*.c -lm -lpthread -o main
