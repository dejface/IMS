CC=g++

all: ims-projekt

ims-projekt.o: ims-projekt.cc
                $(CC) -c ims-projekt.cc -o $@

ims-projekt: ims-projekt.o
                $(CC) ims-projekt.o -o $@

run:
        ./ims-projekt