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

struct cp_state {
	char *ctk;
	int ntk;
	int inc_header;
	int inc_list;
	int lflags;
	int gflags;
};


void parse_line(char*, int, struct cp_state*);

void switch_ctl_tokens(char, struct cp_state*);
void parse_str_tokens(char, struct cp_state*);
void parse_ctl_tokens(struct cp_state*);

void parse_ctl_x2f(struct cp_state*); /* / */
void parse_ctl_x52(struct cp_state*); /* * */
void parse_ctl_x76(struct cp_state*); /* = */ 


void creole_parse(char *text)
{
	printf("%s\n\n-----\n\n", text);
	char ch = '\0';
	int i = 0, flags = 0;
	struct cp_state state;
	state.ntk = 0;
	state.inc_header = 0;
	state.inc_list = 0;
	state.lflags = 0;
	state.gflags = 0;
	/* new lines seem to be a critical token
	 * at the moment. Get each new line */
	while(text[i] != '\0') {
		if(text[i] == '\n') {
			text[i] = '\0';
			parse_line(text, i, &state);
			text += ++i;
			i = 0;
			continue;
		}
		i++;
	}

	if(i)
		parse_line(text, i, &state);

	
	if(state.gflags & CG_UL)
		printf("</ul>");
}

void parse_ctl_tokens(struct cp_state *s)
{
	s->ctk[s->ntk] = '\0';
	if(s->ctk[0] != '=' && ( s->lflags & CL_HEADER || ( s->ntk == 1 && (s->lflags & CL_STR) ) ) ) {
		if((s->lflags & CL_TRAIL))
			printf("{{{");

		printf("%s", s->ctk);
		
		if((s->lflags & CL_TRAIL))
			printf("}}}");

		s->ntk = 0;
		return;
	}

	switch(s->ctk[0]) {
	case '*':
		parse_ctl_x52(s);
	break;

	case '=':
		parse_ctl_x76(s);
	break;

	case '/':
		parse_ctl_x2f(s);
	break;
	}

	s->ntk = 0;
}

/* handle * */
void parse_ctl_x52(struct cp_state *s)
{
	if(s->lflags&CL_STR || !(s->lflags&(CL_STR|CL_CTL))) {
		/* in string mode or space terminated control mode */
		unsigned short f = 0;
		while(s->ntk > 0) {
			if(++f == 2) {
				if(s->gflags & CG_BOLD)
					printf("</strong>");
				else
					printf("<strong>");

				s->gflags ^= CG_BOLD;
				f = 0;
			}
			s->ntk--;
		}
		if(f > 0)
			printf("*");

	} else
	if((s->gflags & CG_UL) && !((s->lflags) & CL_STR)) {
		if(s->inc_list < s->ntk) {
			s->inc_list = s->ntk;
			printf("<ul>\n");
		} else 
		if(s->inc_list > s->ntk) {
			s->inc_list = s->ntk;
			printf("</ul>\n");
		}
		printf("<li>");
	} else
	if(!(s->gflags & CG_UL) && !(s->lflags & CL_STR) ) {
		if(s->ntk == 1) {
			(s->gflags) ^= CG_UL;
			s->inc_list = s->ntk;
			printf("<ul>\n");
			printf("<li>");
		}
	}
}

/* handle = */
void parse_ctl_x76(struct cp_state *s)
{
	if((s->lflags & CL_HEADER) && ((s->lflags) & CL_STR) && (s->lflags & CL_TRAIL))
		return;

	printf("%s", s->ctk);
}

void parse_ctl_x2f(struct cp_state *s)
{
	if(s->lflags&CL_STR) {
		/* in string mode */
		unsigned short f = 0;
		while(s->ntk > 0) {
			if(++f == 2) {
				if(s->gflags & CG_ITALIC)
					printf("</em>");
				else
					printf("<em>");

				s->gflags ^= CG_ITALIC;
				f = 0;
			}
			s->ntk--;
		}
		if(f > 0)
			printf("/");

	}
}

void parse_line(char *line, int len, struct cp_state *s)
{
	int i = 0;
	char ch = '\0';
	s->ctk = malloc(sizeof(char)<<5);
	s->lflags = CL_CTL; /* flag control mode */
	s->inc_header = 0;

	while((ch = line[i++]) != '\0') {

		if(ch>0x30 && ch != '=') {
			parse_str_tokens(ch, s);
			continue;
		}

		switch_ctl_tokens(ch, s);
	}

	if(s->ntk) {
		s->lflags ^= CL_TRAIL;
		parse_ctl_tokens(s);
		s->ntk = 0;
	}

	free(s->ctk);
	if(s->lflags & CL_HEADER)
		printf("</h%d>", s->inc_header);

	if(s->gflags & CG_UL)
		printf("</li>\n");

	printf("\n");
}

void parse_str_tokens(char ch, struct cp_state *s)
{
	if(!(s->lflags&(CL_CTL|CL_STR)) || (s->lflags & CL_CTL) && !(s->lflags & CL_STR)) {
		if(s->ntk) {
			/* parse preceeding control tokens */
			parse_ctl_tokens(s);
			s->ntk = 0;
		}

		/* flag in string mode, out of control mode */
		s->lflags ^= CL_CTL | CL_STR;
	}

	if(( s->lflags & CL_OPEN_HEADER)) {
		/* the control tokens opened a header */
		printf("<h%d>", s->inc_header);
		s->lflags ^= CL_OPEN_HEADER;
		s->lflags ^= CL_HEADER; /* the rest is header mode */
	}

	if(s->ntk) {
		parse_ctl_tokens(s);
		s->ntk = 0;
	}
	printf("%c", ch);
}

void switch_ctl_tokens(char ch, struct cp_state *s)
{
	if(s->ntk > 0 && ch != s->ctk[s->ntk-1]) {
		parse_ctl_tokens(s);
		s->ntk = 0;
	}

	switch(ch) {
	case ' ':
		if(s->lflags & CL_CTL)
			s->lflags ^= CL_CTL;

		if(!(s->lflags & CL_STR))
			break;

		printf(" ");
	break;
	default:
		s->ctk[s->ntk++] = ch;
	break;
	}
}
