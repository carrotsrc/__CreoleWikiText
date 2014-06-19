/*
* Copyright 2014, carrotsrc.org
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "creole_parse.h"
#define TEST_FILE "/home/charlie/development/c/creole/test.cre"
int main(int argc, char *argv[])
{
	unsigned int sz = 0;
	char *text = NULL;
	const char *host = "http://wiki/";

	FILE *fp = fopen(TEST_FILE, "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to open test file\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	text = malloc((sizeof(char)*sz)+1);
	fread(text, sz, 1, fp);
	char *fbuf = NULL;

	fbuf = creole_parse(text, host, sz);
	printf("%s", fbuf);
	printf("\n\n");

	exit(EXIT_SUCCESS);
}
