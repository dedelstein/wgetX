/**
 *  Dan Edelstein
 *
 * daedel@ttu.ee
 *
 * Adapted from code provided by Jiazi Yi and Pierre Pfister
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"url.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Missing argument. Please enter URL.\n");
		return 1; // Different non-0 returns for different errors, as in url.c
	}

	char* url = argv[1];
	urlInfo info;

	int parseReturn = ParseUrl(url, &info);
	if (parseReturn) {
		fprintf(stderr, "Could not parse URL: %s\n", ParseUrlErrstr[parseReturn]);
		return 2; //  Second non-0 return value
	}

	PrintUrlInfo(&info);
	return 0;
}
