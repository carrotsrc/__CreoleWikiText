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
int main(int argc, char *argv[])
{
	unsigned int sz = 0;
	char *text = NULL;

	FILE *fp = fopen("test.cre", "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to open test file\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	text = malloc((sizeof(char)*sz)+1);
	fread(text, sz, 1, fp);

	creole_parse(text, "http://wiki");
	printf("\n\n");

	exit(EXIT_SUCCESS);
}
