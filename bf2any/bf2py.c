#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf2any.h"

/*
 * Python translation from BF, runs at about 18,000,000 instructions per second.
 *
 * There is a limit on the number of nested loops was 20 now 100.
 */

/* #define USESYS // else USEOS */

int ind = 0;
#define I printf("%*s", ind*4, "")
int tapelen = 0;

int
check_arg(const char * arg)
{
    if (strcmp(arg, "-O") == 0) return 1;
    if (strncmp(arg, "-M", 2) == 0) {
	tapelen = strtoul(arg+2, 0, 10) + BOFF;
	return 1;
    }
    return 0;
}

void
outcmd(int ch, int count)
{
    switch(ch) {
    case '!':
	printf("#!/usr/bin/python\n");
#ifdef USESYS
	printf("import sys\n");
#else
	printf("import os\n");
#endif
	if (tapelen>0) {
	    printf("m = [0] * %d\n", tapelen);
	} else {
	    /* Dynamic arrays are 20% slower! */
	    printf("from collections import defaultdict\n");
	    printf("m = defaultdict(int)\n");
	}
	printf("p = %d\n", BOFF);
	break;

    case '=': I; printf("m[p] = %d\n", count); break;
    case 'B':
	if(bytecell) { I; printf("m[p] &= 255\n"); }
	I; printf("v = m[p]\n");
	break;
    case 'M': I; printf("m[p] = m[p]+v*%d\n", count); break;
    case 'N': I; printf("m[p] = m[p]-v*%d\n", count); break;
    case 'S': I; printf("m[p] = m[p]+v\n"); break;
    case 'Q': I; printf("if (v != 0) : m[p] = %d\n", count); break;
    case 'm': I; printf("if (v != 0) : m[p] = m[p]+v*%d\n", count); break;
    case 'n': I; printf("if (v != 0) : m[p] = m[p]-v*%d\n", count); break;
    case 's': I; printf("if (v != 0) : m[p] = m[p]+v\n"); break;

    case 'X': I; printf("raise Exception('Aborting infinite loop')\n"); break;

    case '+': I; printf("m[p] += %d\n", count); break;
    case '-': I; printf("m[p] -= %d\n", count); break;
    case '<': I; printf("p -= %d\n", count); break;
    case '>': I; printf("p += %d\n", count); break;
    case '[':
	if(bytecell) { I; printf("m[p] &= 255\n"); }
	I; printf("while m[p] :\n"); ind++; break;
    case ']':
	if(bytecell) {
	    I; printf("m[p] &= 255\n");
	}
	ind--;
	break;

#ifdef USESYS
    case '.': I; printf("sys.stdout.write(chr(m[p]&255))\n"); break;
    case ',':
	I; printf("c = sys.stdin.read(1);\n");
	I; printf("if c != '' :\n");
	ind++; I; ind--; printf("m[p] = ord(c)\n");
	break;
#else
    case '.': I; printf("os.write(1, chr(m[p]&255))\n"); break;
    case ',':
	I; printf("c = os.read(0, 1);\n");
	I; printf("if c != '' :\n");
	ind++; I; ind--; printf("m[p] = ord(c)\n");
	break;
#endif
    }
}
