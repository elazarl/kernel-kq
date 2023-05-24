.PHONY: all

all: kq

CLFAGS=-fsanitize=address -fsanitize=undefined
CC=clang
