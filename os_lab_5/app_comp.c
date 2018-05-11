#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	struct Array *arr;
	initialize(&arr, 3);
	print(arr);
    unsigned char *output = str2md5("hello", strlen("hello"));
    set(arr, 0, output);
	print(arr);
    output = str2md5("test", strlen("test"));
    set(arr, 1, output);
	print(arr);
    output = str2md5("world", strlen("world"));
    set(arr, 2, output);
	print(arr);
	delete(arr);
    return 0;
 }
