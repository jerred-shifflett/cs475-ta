#!/bin/bash

#number of threads
foreach t (1 2 4 8)
	echo NUMT =$t
	g++ -DNUMT=$t project1.cpp -o project1 --lm -fopenmp
	./prog
end