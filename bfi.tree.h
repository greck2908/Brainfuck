
#include "bfi.conf.h"

extern char * curfile;
extern int curr_line, curr_col;
extern int noheader, enable_trace, hard_left_limit;

#define TOKEN_LIST(Mac) \
    Mac(MOV) Mac(ADD) Mac(PRT) Mac(INP) Mac(WHL) Mac(END) \
    Mac(SET) Mac(CALC) \
    Mac(ZFIND) Mac(MFIND) Mac(ADDWZ) \
    Mac(IF) Mac(ENDIF) Mac(MULT) Mac(CMULT) Mac(FOR) \
    Mac(CALC2) Mac(CALC3) Mac(CALC4) Mac(CALC5) \
    Mac(STOP) Mac(NOP) Mac(DEAD) Mac(ERR)

#define GEN_TOK_ENUM(NAME) T_ ## NAME,
enum token { TOKEN_LIST(GEN_TOK_ENUM) TCOUNT};

struct bfi
{
    struct bfi *next;
    int type;
    int count;
    int offset;
    struct bfi *jmp;

    int count2;
    int offset2;
    int count3;
    int offset3;

    int profile;
    int line, col;
    int inum;
    int ipos;

    int orgtype;
    struct bfi *prev;
    struct bfi *prevskip;
};

extern struct bfi *bfprog;
extern const char* tokennames[];
extern int node_type_counts[TCOUNT+1];
extern int total_nodes;

/* How far to search for constants. */
#define SEARCHDEPTH     10
#define SEARCHRANGE     10000

void calculate_stats(void);
void printtreecell(FILE * efd, int indent, struct bfi * n);

void
find_known_value(struct bfi * n, int v_offset, struct bfi ** n_found,
		int * const_found_p, int * known_value_p, int * unsafe_p);
