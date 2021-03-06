#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf2any.h"
#include "move_opt.h"

/*
 * Ruby translation from BF, runs at about 31,000,000 instructions per second.
 */

static int ind = 0;
#define I printf("%*s", ind*4, "")
static int safetapeoff = 0, curtapeoff = 0;
static int init_done = 0;

static void print_cstring(void);

static check_arg_t fn_check_arg;
struct be_interface_s be_interface = {.check_arg=fn_check_arg};

static int
fn_check_arg(const char * arg)
{
    if (strcmp(arg, "-M") == 0) {
	tapelen = 0;
	return 1;
    }
    return 0;
}

static char *
cell(int mov)
{
    static char buf[6+3+sizeof(mov)*3];
    if (mov == 0) return "m[p]";
    if (mov < 0)
	sprintf(buf, "m[p-%d]", -mov);
    else
	sprintf(buf, "m[p+%d]", mov);
    return buf;
}

void
outcmd(int ch, int count)
{
    int mov = 0;
    char * cm;

    if (tapelen>0) move_opt(&ch, &count, &mov);
    if (ch == 0) return;

    if (!init_done && ch != '!' && ch != '"' && ch != '~')
    {
	/* Using push is about 20% slower, using a Hash is a LOT slower! */
	if (tapelen > 0)
	    printf("m = Array.new(%d, 0)\n",tapesz);
	else {
	    puts("m = Array.new");
	}
	printf("%s%d%s", "p = ", tapeinit, "\n");
	if (tapelen <= 0)
	    puts("m.push(0) while p>=m.length");
	init_done = 1;
    }

    cm = cell(mov);
    switch(ch) {
    case '!':
	puts("#!/usr/bin/ruby");
	break;

    case '=': I; printf("%s = %d\n", cm, count); break;
    case 'B':
	if(bytecell) { I; printf("%s &= 255\n", cm); }
	I; printf("v= %s\n", cm);
	break;
    case 'M': I; printf("%s += v*%d\n", cm, count); break;
    case 'N': I; printf("%s -= v*%d\n", cm, count); break;
    case 'S': I; printf("%s += v\n", cm); break;
    case 'Q': I; printf("%s = %d unless v == 0\n", cm, count); break;
    case 'm': I; printf("%s += v*%d unless v == 0\n", cm, count); break;
    case 'n': I; printf("%s -= v*%d unless v == 0\n", cm, count); break;
    case 's': I; printf("%s += v unless v == 0\n", cm); break;

    case 'X': I; printf("raise 'Aborting Infinite Loop.'\n"); break;

    case '+': I; printf("%s += %d\n", cm, count); break;
    case '-': I; printf("%s -= %d\n", cm, count); break;
    case '<':
	I; printf("p -= %d\n", count);
	curtapeoff -= count;
	break;
    case '>':
	I; printf("p += %d\n", count);
	curtapeoff += count;
	if (tapelen <= 0 && curtapeoff > safetapeoff) {
	    safetapeoff = curtapeoff;
	    I; puts("m.push(0) while p>=m.length");
	}
	break;
    case '[':
	if(bytecell) { I; printf("%s &= 255\n", cm); }
	I; printf("while %s != 0\n", cm);
	ind++;
	curtapeoff = safetapeoff = 0;
	break;
    case ']':
	if (count > 0) {
            I; printf("p += %d\n", count);
        } else if (count < 0) {
            I; printf("p -= %d\n", -count);
	}

	if(bytecell) { I; printf("%s &= 255\n", cm); }
	ind--; I; printf("end\n");
	curtapeoff = safetapeoff = 0;
	break;
    case 'I':
	if(bytecell) { I; printf("%s &= 255\n", cm); }
	I; printf("if %s != 0\n", cm);
	ind++;
	curtapeoff = safetapeoff = 0;
	break;
    case 'E':
	if (count > 0) {
            I; printf("p += %d\n", count);
        } else if (count < 0) {
            I; printf("p -= %d\n", -count);
	}
	ind--; I; printf("end\n");
	curtapeoff = safetapeoff = 0;
	break;
    case '"': print_cstring(); break;
    case '.':
	I;
	if(bytecell) printf("putc %s\n", cm);
	else printf("begin print '' << %s rescue putc %s end\n", cm, cm);
	break;
    case ',':
	I;
	if (bytecell) printf("%s = $stdin.getbyte if !$stdin.eof\n", cm);
	else          printf("%s = $stdin.getc.ord if !$stdin.eof\n", cm);
	break;
    }
}

static void
print_cstring(void)
{
    char * str = get_string();
    char buf[256];
    int gotnl = 0;
    size_t outlen = 0;

    if (!str) return;

    for(;; str++) {
	if (outlen && (*str == 0 || gotnl || outlen > sizeof(buf)-8))
	{
	    buf[outlen] = 0;
	    if (gotnl) {
		buf[outlen-2] = 0;
		I; printf("puts \"%s\"\n", buf);
	    } else {
		I; printf("print \"%s\"\n", buf);
	    }
	    gotnl = 0; outlen = 0;
	}
	if (!*str) break;

	if (*str == '\n') gotnl = 1;
	if (*str == '"' || *str == '\\' || *str == '#') {
	    buf[outlen++] = '\\'; buf[outlen++] = *str;
	} else if (*str >= ' ' && *str <= '~') {
	    buf[outlen++] = *str;
	} else if (*str == '\n') {
	    buf[outlen++] = '\\'; buf[outlen++] = 'n';
	} else if (*str == '\t') {
	    buf[outlen++] = '\\'; buf[outlen++] = 't';
	} else {
	    char buf2[16];
	    int n;
	    sprintf(buf2, "\\%03o", *str & 0xFF);
	    for(n=0; buf2[n]; n++)
		buf[outlen++] = buf2[n];
	}
    }
}
