/*
 * Copyright (c) 2019, Exile Heavy Industries
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of the copyright holder nor the names of its contributors may be used
 *   to endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS
 * LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* 
 * This is meant to be a simple number-crunching utility to help with tracking expenses
 * relative to income. All expenses are able to be given a category to help track if you're 
 * overspending in certain areas. Since this can hold some valuable information, extra care will be taken 
 * to help prevent access to the database without proper authentication, though this is not a prioity,
 * so ensure you take additional methods to secure the database.
 *
 * Ideal invocation should look like: 
 *
 * budget -D budget.db deposit 500 freelance
 *	500 deposited for freelanece, current balance: 2170
 * 
 * budget -D budget.db total food expenses
 *	spent 120 on food this month (2019.03)
 *
 * So some additional parsing of **av will be necessary, but all subcommands should be possible to 
 * shorten to single characters or abbreviations to enable accelerated processing
 */

#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
/* man 3 sha512 for information on these functions */
/* requires -lmd, good chance of being moved to a separate file */
#include <sha512.h>

/* macro definitions */
#define notimp(a) fprintf(stderr,"%s [%s:%u] %s: -%c is not implemented\n", __progname, __FILE__, __LINE__, __func__, a)
#define pdbg() fprintf(stderr, "%s [%s:%u] %s:", __progname, __FILE__, __LINE__, __func__);

/* declaration of external variables */
extern char *__progname;
extern char **environ;
extern bool dbg;

/* Initialize necessary externs */
bool dbg = false;

int readconfig(const char *conffile);
int init_newdb(const char *dbname, const char *key, const char *pass, const char *categories);
int mkexpense_category(const char *dbname, const char *category);
int insert_transaction(const char *dbname, const char *category, int cost);
static void usage(void);

/* 
 * To hopefully support UTF-8 properly, all other characters other than those needed for the **av
 * array should be treated as uint8_t or wchar_t, though wchar_t is less than ideal when it comes 
 * to storing the character/glyph data efficiently
 */
int
main(int ac, char **av) {
	/* declared register as it's going to be used frequently for determining runtime state */
	register int retc, ch;
	retc = 0;
	while ((ch = getopt(ac, av, "hDd:i:k:p:v")) != -1) {
		switch (ch) {
			case 'd':
				notimp(ch);
				break;
			case 'D':
				dbg = true;
				break;
			case 'h':
				usage();
				/* unless an initialization allocation failed retc should not be non-zero now */
				return(retc);
			case 'k':
				/* key to use for the database decryption (asymmetric cipher), may or may not be password protected */
				notimp(ch);
				break;
			case 'p':
				/* password used for the database decryption (symmetric cipher) */
				notimp(ch);
				break;
			default:
				usage();
				return(retc);
		}
	}
	return(retc);
}

static void
usage(void) {
	fprintf(stderr,"%s: Simple personal finance tracker\n"
			"\t-D  Enable debugging printouts\n"
			"\t-d  Specify the budget database to use\n"
			"\t-h  This help message\n"
			"\t-k  Asymmetric decryption key location\n"
			"\t-p  Symmetric decryption password (you probably shouldn't use this)\n"
			,__progname);
}
