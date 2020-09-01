NAME := $(shell basename $(PWD))
export MODULE := M6
all: $(NAME)-64.so $(NAME)-32.so
test: all git
	@rm -f miao.db;
	@gcc -L. main.c -o libkvdb-test-64 -lkvdb-64 -pthread
	@LD_LIBRARY_PATH=. ./libkvdb-test-64

include ../Makefile
