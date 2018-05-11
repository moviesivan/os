#ifndef DYNLIB_H
#define DYNLIB_H

#include <stdlib.h>
#include <stdio.h>

struct Array
{
	unsigned char** Max;
	int Count;
};


void initialize(struct Array **aptr, int size);		
void delete(struct Array *a);						

unsigned char* get(struct Array *a, int i);					
void set(struct Array *a, int i, unsigned char* data);		
void swap(struct Array *a, int i1, int i2);			

void print(struct Array *a);					

#endif
