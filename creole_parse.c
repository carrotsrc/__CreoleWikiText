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

#define LF(f) (s->lflags & (f))
#define GF(f) (s->gflags & (f))

#define TLF(f) s->lflags ^= f
#define TGF(f) s->gflags ^= f
#define CTOK(s, c) (s->ctok[s->nct++] = c)

struct cp_state {
	/* control tokens */
	char *ctk;
	unsigned int nct;

	/* string tokens */
	char *stk;
	unsigned int nst;


	unsigned int inc_header; /* header incerement */
	unsigned int inc_list; /* list increment */
	unsigned int lflags; /* local/line flags */
	unsigned int gflags; /* global flags */
	const char *host; /* wiki host */

	/* output buffer */
	char *fbuf;
	unsigned int flen;
	unsigned int fpos;
};


void parse_line(char*, int, struct cp_state*);

static void switch_ctl_tokens(char, struct cp_state*);
static void parse_str_tokens(char, struct cp_state*);
static void parse_ctl_tokens(struct cp_state*);

static int check_url(const char *, unsigned int);

static void parse_ctl_x2d(struct cp_state*); /* - */
static void parse_ctl_x2f(struct cp_state*); /* / */
static void parse_ctl_x52(struct cp_state*); /* * */
static void parse_ctl_x5b(struct cp_state*); /* [ */ 
static void parse_ctl_x5d(struct cp_state*); /* ] */ 
static void parse_ctl_x76(struct cp_state*); /* = */ 
static void parse_ctl_x7c(struct cp_state*); /* | */ 

static void printbuf_str(struct cp_state*, char*);
static void printbuf_ctok(struct cp_state*);
static void printbuf_stok(struct cp_state*);
static void printbuf_ch(struct cp_state*, char);

static void realloc_buf(struct cp_state*);

void realloc_buf(struct cp_state *s)
{
	s->flen = s->flen+(s->flen>>1);
	char *tmp = (char*)realloc((void*)(s->fbuf),  sizeof(char)*(s->flen+1));
	if(tmp == NULL)
		exit(EXIT_FAILURE);
	s->fbuf = tmp;
	tmp = NULL;
}

void printbuf_str(struct cp_state *s, char *str)
{
	unsigned short i = 0;
	while(str[i] != '\0') {
		s->fbuf[s->fpos] = str[i++];
		if(++s->fpos == s->flen)
			realloc_buf(s);
	}

}

void printbuf_ch(struct cp_state *s, char ch)
{
	if((s->fpos+1) >= s->flen)
			realloc_buf(s);

	s->fbuf[s->fpos++] = ch;

}

void printbuf_ctok(struct cp_state *s)
{
	if((s->fpos+s->nct) >= s->flen)
			realloc_buf(s);

	for(unsigned short i = 0; i < s->nct; i++)
		s->fbuf[s->fpos++] = s->ctk[i];

}

void printbuf_stok(struct cp_state *s)
{
	if((s->fpos+s->nst) >= s->flen)
			realloc_buf(s);

	for(unsigned short i = 0; i < s->nst; i++)
		s->fbuf[s->fpos++] = s->stk[i];

}

char *creole_parse(char *text, const char *host, int len)
{
	unsigned char ch = '\0';
	unsigned int i = 0, flags = 0;
	struct cp_state state;

	/* initialise the machine */
	state.nct = 0;
	state.inc_header = 0;
	state.inc_list = 0;
	state.lflags = 0;
	state.gflags = 0;
	state.host = host;
	state.flen = len+(len>>1);
	state.fbuf = malloc(sizeof(char)*(state.flen+1));

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

	if(i) /* theres stuff left parse */
		parse_line(text, i, &state);

	if(state.gflags & CG_UL)
		printbuf_str(&state, "</ul>");

	printbuf_str(&state, "\n\0");
	return state.fbuf;
}

void parse_line(char *line, int len, struct cp_state *s)
{
	int i = 0;
	char ch = '\0';

	/* init control token buffer for line */
	s->ctk = malloc(sizeof(char)<<5);
	s->nct = 0;

	/* init string token buffer for line */
	s->stk = malloc(sizeof(char)*len);
	s->nst = 0;


	s->lflags = CL_CTL; /* flag control mode */
	s->inc_header = 0;

	while((ch = line[i++]) != '\0') {
		if(ch>0x30  && ch < 0x7b && ch != '=' && ch != '[' && ch != ']') {
			parse_str_tokens(ch, s);
			continue;
		}

		switch_ctl_tokens(ch, s);
	}

	if(s->nst > 0 && !LF(CL_AHREF|CL_ATITLE) ) {
		/* if stuff is there and not a link */
		printbuf_stok(s);
		s->nst = 0;
	}

	if(s->nct) {
		s->lflags ^= CL_TRAIL;
		parse_ctl_tokens(s);
		s->nct = 0;
	}

	free(s->ctk);

	if(LF(CL_HEADER)) {
		char *tmp = malloc(8);
		sprintf(tmp, "</h%d>", s->inc_header);
		printbuf_str(s, tmp);
		free(tmp);
	}

	if(GF(CG_UL))
		printbuf_str(s, "</li>");

	printbuf_str(s, "\n\0");
}

void parse_str_tokens(char ch, struct cp_state *s)
{

	if( !LF(CL_CTL|CL_STR) || (LF(CL_CTL)) && !LF(CL_STR) ) {
		if(s->nct) {
			/* parse preceeding control tokens */
			parse_ctl_tokens(s);
			s->nct = 0;
		}

		if( GF(CG_UL) && !LF(CL_LIST) ) {
			printbuf_str(s, "</ul>\n");
			s->gflags ^= CG_UL;
		}
			
		/* flag in string mode, out of control mode */
		if(LF(CL_CTL))
			s->lflags ^= CL_CTL;

		s->lflags ^= CL_STR;
	}

	if(LF(CL_OPEN_HEADER)) {
		/* the control tokens opened a header */
		s->lflags ^= CL_OPEN_HEADER;
		s->lflags ^= CL_HEADER; /* the rest is header mode */
	}

	if(s->nct) {
		parse_ctl_tokens(s);
		s->nct = 0;
	}

	s->stk[s->nst++] = ch;
}

void switch_ctl_tokens(char ch, struct cp_state *s)
{
	if(s->nst > 0 && !LF(CL_AHREF) ) {

		printbuf_stok(s);
		s->nst = 0;
	}


	if(s->nct > 0 && ch != s->ctk[s->nct-1]) {
		parse_ctl_tokens(s);
		s->nct = 0;
	}

	switch(ch) {
	case ' ':
		if(LF(CL_CTL))
			s->lflags ^= CL_CTL;

		if(LF(CL_ATITLE)) {
			s->stk[s->nst++] = ' ';
			break;
		}
		if(!LF(CL_STR))
			break;

		printbuf_str(s, " ");
	break;

	case '.':
		s->stk[s->nst++] = '.';
	break;

	case '/':
		if(LF(CL_AHREF)) {
			s->stk[s->nst++] = '/';
			break;
		}
		
		s->ctk[s->nct++] = ch;
	break;

	default:
		s->ctk[s->nct++] = ch;
	break;
	}
}

void parse_ctl_tokens(struct cp_state *s)
{
	s->ctk[s->nct] = '\0';
	if(s->ctk[0] != '=' && s->ctk[0] != '|' && ( LF(CL_HEADER) || (s->nct == 1 && LF(CL_STR)) ) ) {

		printbuf_ctok(s);

		s->nct = 0;
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

	case '-':
		parse_ctl_x2d(s);
	break;

	case '[':
		parse_ctl_x5b(s);
	break;

	case ']':
		parse_ctl_x5d(s);
	break;

	case '|':
		parse_ctl_x7c(s);
	break;
	}

	s->nct = 0;
}

/* handle * 
 *
 * this one is a pain because it deals with both
 * strong emphasis and unordered list
 * */
void parse_ctl_x52(struct cp_state *s)
{
	if(( LF(CL_STR) || !LF(CL_STR|CL_CTL) ) ||
		(!GF(CG_UL) && LF(CL_CTL) && s->nct > 1) ) {
		/* in string mode or space terminated control mode  OR
		 * not in unordered list AND not in String mode AND ntok > 1 */
		unsigned short f = 0;
		while(s->nct > 0) {
			if(++f == 2) {
				if(s->gflags & CG_BOLD)
					printbuf_str(s, "</strong>");
				else
					printbuf_str(s, "<strong>");

				s->gflags ^= CG_BOLD;
				f = 0;
			}
			s->nct--;
		}
		if(f > 0)
			printbuf_str(s, "*");

	} else
	if(GF(CG_UL) && LF(CL_CTL)) {
		/* in UL mode and control mode */
		if(s->inc_list < s->nct) {
			s->inc_list = s->nct;
			printbuf_str(s, "<ul>\n");
		} else 
		if(s->inc_list > s->nct) {
			s->inc_list = s->nct;
			printbuf_str(s, "</ul>\n");
		}

		s->lflags ^= CL_LIST;
		printbuf_str(s,"<li>");
	} else
	if(!GF(CG_UL) && LF(CL_CTL) && s->nct == 1) {
		/* NOT in string more OR UL mode and ntok = 1
		 * so we must be starting an unordered list*/
		s->gflags ^= CG_UL;
		s->inc_list = s->nct;
		printbuf_str(s, "<ul>\n");
		printbuf_str(s, "<li>");
		s->lflags ^= CL_LIST;
	}
}

/* handle = */
void parse_ctl_x76(struct cp_state *s)
{
	if(s->lflags == CL_CTL) {
		s->inc_header = s->nct;

		char *tmp = malloc(8);
		sprintf(tmp, "<h%d>", s->inc_header);
		printbuf_str(s, tmp);

		free(tmp);
		s->lflags ^= CL_OPEN_HEADER;
		return;
	}

	if(LF(CL_HEADER) && LF(CL_STR) && LF(CL_TRAIL))
		return;

	printbuf_ctok(s);
}

/* handle / */
void parse_ctl_x2f(struct cp_state *s)
{
	if(LF(CL_STR) || LF(CL_CTL)) {
		/* in string mode */
		unsigned short f = 0;
		while(s->nct > 0) {
			if(++f == 2) {
				if(GF(CG_ITALIC))
					printbuf_str(s, "</em>");
				else
					printbuf_str(s, "<em>");

				s->gflags ^= CG_ITALIC;
				f = 0;
			}
			s->nct--;
		}
		if(f > 0)
			printbuf_str(s, "/");

	}
}

/* handle - */
void parse_ctl_x2d(struct cp_state *s)
{
	unsigned short chk = CL_CTL|CL_TRAIL;
	if(LF(chk) == chk && s->nct >= 4)
		printbuf_str(s, "<hr />");
	else
		printbuf_ctok(s);
}

/* handle [ */
void parse_ctl_x5b(struct cp_state *s)
{
	if(s->nct > 1 && !LF(CL_AHREF|CL_ATITLE) ) {
		unsigned short f = s->nct - 2;
		
		printbuf_str(s, "<a href=\"");
		while(f-- > 0)
			printbuf_str(s, "[");

		s->lflags ^= CL_AHREF;
		s->nct = 0;

		return;
	}

	printbuf_ctok(s);
}

/* handle ] */
void parse_ctl_x5d(struct cp_state *s)
{
	
	if(!LF(CL_AHREF|CL_ATITLE) )
		printbuf_ctok(s);

	if(s->nct > 1) {
		unsigned short f = s->nct - 2;
		if(LF(CL_AHREF)) {
			/* it's a single style internal link */
			char *tmp = malloc(sizeof(char*)*(strlen(s->host)+s->nst+1));
			s->stk[s->nst] = '\0';
			sprintf(tmp, "%s%s\">%s</a>", s->host, s->stk, s->stk);
			printbuf_str(s, tmp);

			free(tmp);
			s->lflags ^= CL_AHREF;
			s->nst = 0;
		} else
		if(LF(CL_ATITLE)) {
			/* it's a double style link */
			printbuf_str(s, ">");
			s->stk[s->nst] = '\0';
			printbuf_str(s, s->stk);
			printbuf_str(s, "</a>");
			s->lflags ^= CL_ATITLE;
			s->nst = 0;
		}
		
		while(f-- > 0)
			printbuf_str(s, "]");

		s->nct = 0;

		return;
	}

}

/* handle | */
void parse_ctl_x7c(struct cp_state *s)
{
	if(!LF(CL_AHREF))
		printbuf_ctok(s);

	s->lflags ^= CL_AHREF;
	s->lflags ^= CL_ATITLE;

	if(s->nst < 8 || !check_url(s->stk, s->nst)) {
		char *tmp = malloc(sizeof(char*)*(strlen(s->host)+s->nst+1));
		s->stk[s->nst] = '\0';
		sprintf(tmp, "%s%s", s->host, s->stk);
		printbuf_str(s, tmp);
		printbuf_str(s, "\">");
	} else {
		printbuf_stok(s);
		printbuf_str(s, "\">");
	}
	s->nst = 0;
}

/* Basic check to see if we're dealing with a URL */
int check_url(const char *str, unsigned int len)
{
	if(len < 8)
		return 0;
	
	const char url[8] = "http://";
	unsigned short i = 0;
	while(i < 7)
		if(str[i] != url[i++])
			return 0;
	return 1;
}
