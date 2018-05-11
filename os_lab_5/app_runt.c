#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <openssl/md5.h>
#include "dynlib.h"

unsigned char *str2md5(const char *str, int length) {
    MD5_CTX c;
    unsigned char *digest = (unsigned char*)malloc(16);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    return digest;
}

int main(int argc, char **argv) {
	struct Array *arr;void *lib_handle;
	void (*init)(struct Array **, int);
	void (*print1)(struct Array *);
	void (*set1)(struct Array *,int, unsigned char *);
	void (*delete1)(struct Array *);
	char *error;
 
	lib_handle = dlopen("./libdynl.so", RTLD_LAZY);
	if (!lib_handle)
	{
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}
 
	init = dlsym(lib_handle, "initialize");
	if ((error = dlerror()) != NULL) 
	{
		fprintf(stderr, "%s\n", error);
		exit(1);
	}
	print1 = dlsym(lib_handle, "print");
	if ((error = dlerror()) != NULL) 
	{
		fprintf(stderr, "%s\n", error);
		exit(1);
	}
	set1 = dlsym(lib_handle, "set");
	if ((error = dlerror()) != NULL) 
	{
		fprintf(stderr, "%s\n", error);
		exit(1);
	}
	delete1 = dlsym(lib_handle, "delete");
	if ((error = dlerror()) != NULL) 
	{
		fprintf(stderr, "%s\n", error);
		exit(1);
	}
 
	(*init)(&arr, 3);

	(*print1)(arr);
    unsigned char *output = str2md5("hello", strlen("hello"));
    (*set1)(arr, 0, output);
	(*print1)(arr);
    output = str2md5("test", strlen("test"));
    (*set1)(arr, 1, output);
	(*print1)(arr);
    output = str2md5("world", strlen("world"));
    (*set1)(arr, 2, output);
	(*print1)(arr);
	(*delete1)(arr);
	
	dlclose(lib_handle);
	
    return 0;
 }
