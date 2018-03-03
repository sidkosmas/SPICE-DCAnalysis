# SPARCE DC Analysis

A DC Analysis of SPICE files using sparse matrices and the GSL library written in C.

## 
	gcc -Wall -I/usr/local/include -c *.c
	gcc -g -L/usr/local/lib *.o -lgsl -lgslcblas -lm
## 
	./a spice_file.txt