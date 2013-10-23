
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "bfi.tree.h"

static void
pint(int v)
{
    if (v<0) printf("(%d)", -v);
    else     printf("%d", v);
}

/*
 * Regenerate some BF code.
 *
 * Most optimised tokens cannot be generated because the code would need
 * temp cells on the tape, but the information as to which temps were used
 * has been discarded. However, we can use any cell who's value is known.
 *
 * You may have to reduce the optimisation to -O1
 */
void 
print_bf(void)
{
    struct bfi *v, *n = bfprog;
    int last_offset = 0;
    int i, nocr = 0;

    while(n)
    {
	if (enable_trace && !nocr) { /* Sort of! */
	    v = n;

	    do 
	    {
		printf("// %s", tokennames[v->type&0xF]);
		printf(" O="); pint(v->offset);
		switch(v->type) {
		case T_WHL: case T_ZFIND: case T_ADDWZ: case T_MFIND:
		    printf(" ID="); pint(v->count);
		    break;
		case T_END: case T_ENDIF:
		    printf(" ID="); pint(v->jmp->count);
		    if (v->count != 0) { printf(" C="); pint(v->count); }
		    break;
		case T_ADD:
		    printf(" V="); pint(v->count);
		    break;
		case T_SET:
		    printf(" ="); pint(v->count);
		    break;
		default:
		    printf(" C="); pint(v->count);
		    break;
		}
		if (v->count2 != 0) {
		    printf(" O2="); pint(v->offset2);
		    printf(" C="); pint(v->count2);
		}
		if (v->count3 != 0) {
		    printf(" O3="); pint(v->offset3);
		    printf(" C="); pint(v->count3);
		}
		printf(" @%d;%d", v->line, v->col);
		printf("\n");
		v=v->next;
	    }
	    while(v && v->prev->type != T_END && v->prev->type != T_ENDIF && (
		n->type == T_MULT ||
		n->type == T_CMULT ||
		n->type == T_ADDWZ ||
		n->type == T_IF ||
		n->type == T_FOR));
	}

	if (n->type == T_MOV) {
	    last_offset -= n->count;
	    n = n->next;
	    continue;
	}

	if (n->offset!=last_offset) {
	    while(n->offset>last_offset) { putchar('>'); last_offset++; };
	    while(n->offset<last_offset) { putchar('<'); last_offset--; };
	    if (!nocr)
		printf("\n");
	}

	switch(n->type)
	{
	case T_SET:
	    printf("[-]");
	    i = n->count;
	    while(i>0) { putchar('+'); i--; }
	    while(i<0) { putchar('-'); i++; }
	    break;

	case T_ADD:
	    i = n->count;
	    while(i>0) { putchar('+'); i--; }
	    while(i<0) { putchar('-'); i++; }
	    break;

	case T_PRT:
	    if (n->count != -1) {
		int const_found=0, known_value, non_zero_unsafe, offset = n->offset;
		/*
		 *  Check to see if the value is known; it was known but the
		 *  change may have been moved or the cell is now left at it's
		 *  old value. If the value is known we put it back after we've
		 *  used it, if not it must be reset later.
		 */
		{
		    offset = n->offset;
		    find_known_value(n->prev, offset,
			0, &const_found, &known_value, &non_zero_unsafe);
		}

		i = offset;
		while(i>last_offset) { putchar('>'); last_offset++; };
		while(i<last_offset) { putchar('<'); last_offset--; };
		printf("[-]");
		i = n->count;
		while(i>0) { putchar('+'); i--; }
		while(i<0) { putchar('-'); i++; }
		putchar('.');
//		if(n->next && n->next->type == T_PRT && n->next->count != -1);

		if (const_found) {
		    i = known_value-n->count;
		    while(i>0) { putchar('+'); i--; }
		    while(i<0) { putchar('-'); i++; }
		}
	    } else
		putchar('.');
	    break;

	case T_INP:
	    putchar(',');
	    break;

	case T_ZFIND:
	    if (n->next->next != n->jmp) {
		putchar('[');
	    } else {
		int i;
		putchar('[');
		if (n->next->count >= 0) {
		    for(i=0; i<n->next->count; i++)
			putchar('>');
		} else {
		    for(i=0; i< -n->next->count; i++)
			putchar('<');
		}
		putchar(']');
		n=n->jmp;
	    }
	    break;

	case T_MULT: case T_CMULT:
	case T_ADDWZ: case T_IF: case T_FOR:
	case T_MFIND:
	    nocr = 1;
	case T_WHL:
	    putchar('[');
	    // BFBasic
	    // if (n->offset == 2) printf("\n> [-]>[-]>[-]>[-]>[-]>[-]>[-]><<<<<<< <");
	    break;

	case T_END:
	    putchar(']');
	    nocr = 0;
	    break;

	case T_ENDIF:
	    printf("[-]]");
	    nocr = 0;
	    break;

	case T_STOP:
	    printf("[-]+[]");
	    nocr = 0;
	    break;

	case T_NOP: 
	    fprintf(stderr, "Warning on code generation: "
	           "NOP node: ptr+%d, cnt=%d, @(%d,%d).\n",
		    n->offset, n->count, n->line, n->col);
	    break;

	default:
	    fprintf(stderr, "Error on code generation:\n"
	           "Bad node: %s ptr+%d, cnt=%d.\n",
		    tokennames[n->type], n->offset, n->count);
	    fprintf(stderr, "Optimisation level probably too high for BF output\n");
	    exit(1);
	}
	if (!nocr)
	    putchar('\n');
	n=n->next;
    }
}
