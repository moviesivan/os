#include <stdlib.h>
#include <stdio.h>
#include "dynlib.h"						

void initialize(struct Array **aptr, int size)
{
	(*aptr) = (struct Array*) malloc( sizeof(struct Array) );
	
	(*aptr)->Max = (unsigned char**) malloc( sizeof(char*) * size);
	
	for(int j = 0; j < size; j++) (*aptr)->Max[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
	
	(*aptr)->Count = size;
	
};

void delete(struct Array *a)
{
	for(int j = 0; j < a->Count; j++) free(a->Max[j]);
	free(a->Max);
	free(a);
};



unsigned char* get(struct Array *a, int i)
{
	if (i > -1 && i < a->Count)
		return a->Max[i];
	else
		return NULL;
};

void set(struct Array *a, int i, unsigned char* data)
{
	if (i > -1 && i < a->Count){
		free(a->Max[i]);
		a->Max[i] = data;
	}
};

void swap(struct Array *a, int i1, int i2)
{
	if (i1 > -1 && i1 < a->Count && i2 > -1 && i2 < a->Count)
	{
		unsigned char* n = a->Max[i1];
		a->Max[i1] = a->Max[i2];
		a->Max[i2] = n;
	}
};


void print(struct Array *a)
{
	int i;
	for (i = 0; i < a->Count; ++i)
	{
		unsigned char* c;
		c = get(a, i);
		for(int j = 0; j < 16; j++) printf("%02x", c[j]);

		if (i != a->Count - 1) printf(", ");
	}
	printf("\n");
}

