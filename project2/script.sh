#!bin/bash
#number of threads

for t in 1 2 4
do
	echo NUMT =$t
	#number of nodes
	for n in 10 100 1000 5000 10000 20000 400000 
	do 
		echo NUMNODES =$t 
		g++ -o project2 project2.cpp -DNUM=$t -DNUMNODES=$n -lm -fopenmp
		./project2
	done
done