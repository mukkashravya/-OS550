
final: assignment.o
	gcc -o final assignment.o

assignment.o: smukka1_assignment1.c
	gcc -c -o assignment.o smukka1_assignment1.c

clean:
	rm *.o final





