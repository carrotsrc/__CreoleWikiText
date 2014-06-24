#ifndef CREOLE_DEF_H
#define CREOLE_DEF_H

#define CG_BOLD 1
#define CG_ITALIC 2

#define CG_UL 8
#define CG_OL 16

#define CG_NFO 128

#define CL_CTL 1
#define CL_STR 2
#define CL_TRAIL 4

#define CL_OPEN_HEADER 8
#define CL_HEADER 16
#define CL_LIST 32

#define CL_AHREF 64
#define CL_ATITLE 128

#define CL_ISRC 256
#define CL_IALT 512

#define LF(f) (s->lflags & (f))
#define GF(f) (s->gflags & (f))

#define LS_UL 1
#define LS_OL 2

#define MAX_NESTED 32

struct cp_state;

void parse_line(char*, int, struct cp_state*);

static void switch_ctl_tokens(char, struct cp_state*);
static void parse_str_tokens(char, struct cp_state*);
static void parse_ctl_tokens(struct cp_state*);

static int check_url(const char *, unsigned int);

static void ls_push(unsigned short, struct cp_state*);
static void ls_pop(unsigned short, struct cp_state*);

static void parse_ctl_x0a(struct cp_state*); /* \n */
static void parse_ctl_x23(struct cp_state*); /* # */
static void parse_ctl_x2a(struct cp_state*); /* * */
static void parse_ctl_x2d(struct cp_state*); /* - */
static void parse_ctl_x2f(struct cp_state*); /* / */
static void parse_ctl_x5b(struct cp_state*); /* [ */ 
static void parse_ctl_x5c(struct cp_state*); /* [ */ 
static void parse_ctl_x5d(struct cp_state*); /* ] */ 
static void parse_ctl_x76(struct cp_state*); /* = */ 
static void parse_ctl_x7bd(struct cp_state*); /* { } */ 
static void parse_ctl_x7c(struct cp_state*); /* | */ 

static void printbuf_str(struct cp_state*, char*);
static void printbuf_ctok(struct cp_state*);
static void printbuf_stok(struct cp_state*);
static void printbuf_ch(struct cp_state*, char);

static void parse_nowiki(struct cp_state*);
static void parse_img(struct cp_state*);

static void realloc_buf(struct cp_state*);

#endif
