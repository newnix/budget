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

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
/* man 3 sha512 for information on these functions */
/* requires -lmd, good chance of being moved to a separate file */
#include <sha512.h>

/* macro definitions */
#ifndef __EXILE_BUDGET_H
#include "budget.h"
#endif
#ifndef __BUDGETCONF_H
#include "budgetconf.h"
#endif

/* declaration of external variables */
extern char *__progname;
extern char **environ;
extern bool dbg;

/* Initialize necessary externs */
bool dbg = false;

/* Only function specific to this file aside from main() */
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
	uint8_t flags;
	char *dbname, *cfgfile, *enckey, *initfile;
	retc = 0;
	flags = NOMASK;
	while ((ch = getopt(ac, av, "hDd:ik:v:f:C:")) != -1) {
		switch (ch) {
			case 'C':
				flags |= HAVCFG;
				notimp(ch);
				break;
			case 'd':
				flags |= HAVEDB;
				notimp(ch);
				break;
			case 'D':
				dbg = true;
				break;
			case 'h':
				flags &= NOMASK;
				flags |= HELPME;
				usage();
				/* unless an initialization allocation failed retc should not be non-zero now */
				return(retc);
			case 'k':
				flags |= HAVKEY;
				/* key to use for the database decryption (asymmetric cipher), may or may not be password protected */
				notimp(ch);
				break;
			default:
				flags &= NOMASK;
				flags |= HELPME;
				usage();
				return(retc);
		}
	}
	ac -= optind;
	av += optind;
	retc = cook(dbname, initfile, cfgfile, enckey, flags);
	return(retc);
}

static void
usage(void) {
	fprintf(stderr,"%s: Simple personal finance tracker\n"
			"\t%s [flags] [command] [args]\n"
			"Flags:\n"
			"\t-D  Enable debugging printouts\n"
			"\t-d  Specify the budget database to use (Default: %s/.local/%s\n"
			"\t-h  This help message\n"
			"\t-k  Asymmetric decryption key location\n"
			"Commands:\n"
			"\t...Still Loading...\n"
			,__progname, __progname, getenv("HOME"), __progname);
}

int
cook(const char *dbname, const char *sqlfile, const char *cfgfile, const char *enckey, uint8_t flags) {
	int retc;
	sqlite3 *dbptr;
	retc = 0;
	dbptr = NULL;

	/* ensure that we read the config file if provided */
	if ((flags & HAVCFG) == HAVCFG) {
		retc = readconfig(cfgfile);
		/* clear the HAVCFG bit after successful processing */
		flags = (retc == 0) ? flags ^ HAVCFG : flags;
	}

	/* Branch off based on flag value */
	switch (flags & CKMASK) {
		case HAVEDB:
			retc = connect(dbname, dbptr);
			break;
		case HAVKEY|HAVEDB:
			retc = decrypt(dbname, enckey);
			break;
		case INITOK:
			retc = initialize(dbname, dbptr, sqlfile);
		default:
			/* Something has gone wrong */
			fprintf(stderr, "ERR: %s [%s:%u] %s: This should not be possible\n", 
					__progname, __FILE__, __LINE__, __func__);
			return(-1);
	}
	return(retc);
}

/* read in the configuration file if provided */
int
readconfig(const char *conffile) {
	int retc, cfd;
	dbconfig *dbdata;
	cfd = retc = 0;
	dbdata = NULL;

	if ((dbdata = calloc((size_t)1, sizeof(dbconfig))) == NULL) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n", 
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return(1);
	}
	if ((cfd = open(conffile, O_RDONLY|O_CLOEXEC)) < 0 ) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		if (errno == ENOENT) {
			sparseconfig(conffile);
			return(2);
		}
		parseconfig(&cfd, dbdata);
	}
	/* now that we have an open fd, read the file, likely k/v format */
	cfree(dbdata, sizeof(dbconfig));
	close(cfd);
	return(retc);
}

int
connect(const char *dbname, sqlite3 *dbptr) {
	int retc;
	struct stat dbstat;
	retc = 0;
	if (dbname == NULL) {
		retc = -1;
	}
	if ((retc = stat(dbname, &dbstat)) != 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		fprintf(stderr, "Ensure that %s is an initialized budget database\n", dbname);
		return(retc);
	}
	if (dbptr != NULL) {
		retc = -1;
		fprintf(stderr, "ERR: %s [%s:%u] %s: This should not be possible\n",
				__progname, __FILE__, __LINE__, __func__);
	}
	/* causes linker failure for some reason */
	//if ((retc = sqlite3_open_v2(dbname, &dbptr, SQLITE_OPEN_READWRITE, NULL)) != 0) {
	//	return(retc);
	//}
	return(retc);
}

int 
decrypt(const char *dbname, const char *enckey) {
	int retc;
	struct stat dbfile, keyfile;
	retc = 0;
	/* simple tests to ensure passed data actually exists */
	if ((retc = stat(dbname, &dbfile)) != 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return(retc);
	}
	if ((retc = stat(enckey, &keyfile)) != 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return(retc);
	}
	return(retc);
}

int
initialize(const char *dbname, sqlite3 *dbptr, const char *sqlfile) {
	int retc;
	struct stat dbfile, sqlstat;
	retc = 0;
	/* sanity checks on data passed to function */
	if ((retc = stat(dbname, &dbfile)) != 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return(retc);
	}
	if ((retc = stat(sqlfile, &sqlstat)) != 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return(retc);
	}
	if (dbptr != NULL) {
		retc = -1;
		fprintf(stderr, "ERR: %s [%s:%u] %s: Database handle is not NULL, are you sure %s needs to be initialized?\n",
				__progname, __FILE__, __LINE__, __func__, dbname);
	}
	return(retc);
}
