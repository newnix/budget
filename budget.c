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
/* may not actually be necessary for these functions */
#include <sys/mman.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* macro definitions */
#ifndef __EXILE_BUDGET_H
#include "budget.h"
#endif
/* */
#ifndef __EXILE_BUDGETCONF_H
#include "budgetconf.h"
#endif

/* Flags */
#define NOMASK 0x00 /* 0000 0000 */
#define INITDB 0x01 /* 0000 0001 */
#define HAVEDB 0x02 /* 0000 0010 */
#define HAVSQL 0x03 /* 0000 0100 */
#define CONINT 0x08 /* 0000 1000 */
#define HELPME 0x10 /* 0001 0000 */
#define HAVKEY 0x20 /* 0010 0000 */
#define HVPASS 0x40 /* 0100 0000 */
#define HAVCFG 0x80 /* 1000 0000 */
#define INITOK 0x07 /* 0000 0111 */
#define CKMASK 0xFF /* 1111 1111 */


/* declaration of external variables */
extern char *__progname;
extern char **environ;
extern bool dbg;
extern bool noop;

/* Initialize necessary externs */
bool dbg = false;
bool noop = false;

/* Only function specific to this file aside from main() */
static void usage(void);

/* 
 * In order to properly support UTF-8, I'll most likely need the ICU library or similar for
 * encoding/decoding non-ASCII text. This is intended to be supported later on, after at least the core
 * logic is set up and verified as functional.
 */
int
main(int ac, char **av) {
	/* declared register as it's going to be used frequently for determining runtime state */
	register int retc, ch;
	uint8_t flags;
	char *dbname, *cfgfile, *enckey, *initfile;
	retc = 0;
	flags = NOMASK;
	while ((ch = getopt(ac, av, "hDId:ik:vf:C:")) != -1) {
		switch (ch) {
			case 'C':
				/* Config file, overrides defaults */
				flags |= HAVCFG;
				notimp(ch);
				break;
			case 'D':
				/* Used to signal that runtime tracing printouts are desired */
				dbg = true;
				break;
			case 'I':
				/* Initialize the database */
				notimp(ch);
				flags |= INITDB;
				break;
			case 'd':
				/* Database file, overrides default */
				flags |= HAVEDB;
				notimp(ch);
				break;
			case 'f':
				/* SQL file to read from or write to */
				notimp(ch);
				flags |= HAVSQL;
				break;
			case 'h':
				/* Do not force early termination, allow main() to cleanup properly */
				flags = HELPME;
				usage();
				break;
			case 'i':
				/* User wants to run an interactive session, may need a library like editline */
				notimp(ch);
				flags |= CONINT;
				break;
			case 'k':
				/* key to use for the database decryption (asymmetric cipher), may or may not be password protected */
				flags |= HAVKEY;
				notimp(ch);
				break;
			case 'n':
				/* Do not actually write anything, especially used for runtime tracing */
				noop = true;
				notimp(ch);
				break;
			case 'v':
				/* TODO: Look at migration from bitmap flag values and a way to reliably check more han just the database file header */
				/* Verify database headers look correct, may include checksums in later versions */
				notimp(ch);
				break;
			default:
				notimp(ch);
				/* XXX: This may not actually be faster that the usual means of storing a variable */
				flags &= NOMASK;
				flags |= HELPME;
				usage();
				break;
		}
	}
	/* Test to see if HELPME is set */
	if ((flags & HELPME) != HELPME) {
		av += optind;
		retc = cook(dbname, initfile, cfgfile, enckey, av, flags);
	}
	/* Clean up after any dynamic allocations made */
	return(retc);
}

static void
usage(void) {
	fprintf(stderr,"%s: Simple personal finance tracker\n"
			"\t%s [flags] [command] [args]\n"
			"Flags:\n"
			"\t-C  Specify the configuration file to use\n"
			"\t-D  Enable debugging printouts\n"
			"\t-I  Bootstrap the database for use in budgeting\n"
			"\t-d  Specify the budget database to use (Default: %s%s/%s)\n"
			"\t-f  Specify a SQL file to use in bootstrap/interchange functions\n"
			"\t-h  This help message\n"
			"\t-i  Open the database for interactive use\n"
			"\t-k  Asymmetric decryption key location\n"
			"\t-v  Validate the database integrity\n"
			"Commands:\n"
			"\t...Still Loading...\n"
			,__progname, __progname, DEFAULT_BUDGET_PARENTDIR, DEFAULT_BUDGET_DIR, DEFAULT_BUDGET_DB);
}

/* 
 * Determine if we can continue to another function, and pass necessary data to continue processing 
 */
int
cook(const char *dbname, const char *sqlfile, const char *cfgfile, const char *enckey, char **argstr, uint8_t flags) {
	int retc, sqlfd;
	sqlite3 *dbptr;
	retc = sqlfd = 0;
	dbptr = NULL;

	if (dbg) {
		nxentr();
	}
	/* ensure that we read the config file if provided */
	if ((flags & HAVCFG) == HAVCFG) {
		retc = readconfig(cfgfile);
		/* clear the HAVCFG bit after successful processing */
		flags = (retc == 0) ? flags ^ HAVCFG : flags;
	}

	/* Branch off based on flag value */
	switch (flags & CKMASK) {
		case HAVEDB:
			if (connect(dbname, dbptr) == 0) {
				/* TODO: Pass to a function that either accepts or generates a transaction control structure */
				if (argstr != NULL) {
					retc = parsecmd(argstr,dbptr);
				}
			}
			break;
		case HAVKEY|HAVEDB:
			retc = decrypt(dbname, enckey);
			break;
		case INITOK:
			if ((retc = opensql(sqlfile, &sqlfd)) == 0) {
				retc = initialize(dbptr, &sqlfd);
				/* ensure the file descriptor is actually closed */
				close(sqlfd);
			}
		case HAVKEY|INITOK:
			/* Bootstrap encrypted database */
			nxwrn("The encrypted bootstrap process is not yet implemented");
			break;
		default:
			/* Something has gone wrong */
			nxerr("Something has gone horribly wrong!");
			retc = -1;
	}
	if (dbg) {
		nxexit();
	}
	return(retc);
}

/* 
 * read in the configuration file if provided 
 * May need to be rewoked to either set global vars or return a pointer for the 
 * configuration data that gets copied into the transaction control structure
 */
int
readconfig(const char *conffile) {
	int retc, cfd;
	dbconfig *dbdata;
	cfd = retc = 0;
	dbdata = NULL;

	if (dbg) {
		nxentr();
	}
	/* XXX: Does this need dynamic allocation? */
	if ((dbdata = calloc((size_t)1, sizeof(dbconfig))) == NULL) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s\n", 
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		retc = 1;
	}
	if (retc == 0 && (cfd = open(conffile, O_RDONLY|O_CLOEXEC)) < 0 ) {
		nxerr(strerror(errno));
		if (errno == ENOENT) {
			/* Default config does not exist, create it */
			sparseconfig(conffile);
		}
		parseconfig(&cfd, dbdata);
	}
	/* now that we have an open fd, read the file, likely k/v format */
	if (dbg) {
		nxexit();
	}
	/* Ensure we don't try to free unallocated space */
	if (dbdata == NULL) { cfree(dbdata, sizeof(dbconfig)); }
	if (cfd != 0) { close(cfd); }
	return(retc);
}

/*
 * Opens the database for use in other functions
 */
int
connect(const char *dbname, sqlite3 *dbptr) {
	int retc;
	struct stat dbstat;
	retc = 0;

	if (dbg) {
		nxentr();
	}
	if (dbname == NULL) {
		retc = -1;
	}
	if ((retc = stat(dbname, &dbstat)) != 0) {
		nxerr(strerror(errno));
		fprintf(stderr, "ERR: %s [%s:%u] %s: Ensure that %s is an initialized budget database\n", __progname,__FILE__,__LINE__,__func__,dbname);
		return(retc);
	}
	if (dbptr != NULL) {
		retc = -1;
		nxerr("This should not have been possible");
	}
	/* XXX: The shared caching mode may not be of any benefit, revisit later on in development */
	if ((retc = sqlite3_open_v2(dbname, &dbptr, SQLITE_OPEN_READWRITE|SQLITE_OPEN_NOMUTEX|SQLITE_OPEN_SHAREDCACHE, NULL)) != SQLITE_OK) {
		nxerr(sqlite3_errstr(retc));
	}
	if (dbg) {
		nxexit();
	}
	return(retc);
}

/*
 * This function is to decrypt the database file itself, not the contents of the database
 * TODO: Look into how to do this without writing a cleartext database to disk
 * TODO: Move to another file to handle crypto functions
 */
int 
decrypt(const char *dbname, const char *enckey) {
	int retc;
	struct stat dbfile, keyfile;
	retc = 0;

	if (dbg) {
		nxentr();
	}
	/* simple tests to ensure passed data actually exists */
	if ((retc = stat(dbname, &dbfile)) != 0) {
		nxerr(strerror(errno));
		if (dbg) {nxexit();}
		return(retc);
	}
	if ((retc = stat(enckey, &keyfile)) != 0) {
		nxerr(strerror(errno));
		if (dbg) {nxexit();}
		return(retc);
	}
	if (dbg) {nxexit();}
	return(retc);
}

/* 
 * This runs the database initialization after other resources are verified
 * XXX: Will need to verify initialization loop works properly
 * TODO: Add in runtime tracing 
 */
int
initialize(sqlite3 *dbptr, int *sqlfd) {
	int retc;
	struct stat sqlstat;
	char *sqlinit, *sqlstart; 
	const char *sqltail;
	sqlite3_stmt *budgetq;
	retc = 0;
	sqlinit = NULL; sqlstart = NULL; sqltail = NULL;

	if (dbg) {
		nxentr();
	}

	/* Check to ensure we don't have NULL pointers */
	if ((dbptr == NULL) || (sqlfd == NULL)) {
		nxerr("Passed bad pointers!");
		retc = -1;
	}
	if ((retc = fstat(*sqlfd, &sqlstat)) == -1) {
		nxerr(strerror(errno));
	}
	if (retc == 0) {
		/* If retc has not been set to a nonzero number, map the initialization file to build the database */
		if ((sqlinit = mmap(NULL,(size_t)sqlstat.st_size,PROT_READ,MAP_PRIVATE,*sqlfd,(off_t)0)) != NULL) {
			/* Now start the read/update loop after recording the start of mapped data */
			sqlstart = sqlinit; /* recorded to ensure munmap() is called on the start of mapped memory */
			nxinf("Initializing budgeting database...");
			for (;sqlinit < (sqlstart + sqlstat.st_size); sqlinit = (char * const)sqltail) {
				retc = sqlite3_prepare_v2(dbptr,sqlinit,-1,&budgetq,&sqltail); /* pass -1 as length to read up to NULL terminator */
				retc = sqlite3_step(budgetq);
				if (retc != SQLITE_DONE) {
					nxerr(sqlite3_errstr(retc));
				}
				sqlite3_finalize(budgetq);
			}
		}
	}

	if (dbg) {
		nxexit();
	}
	/* Close the database connection */
	sqlite3_close(dbptr);
	return(retc);
}

/* 
 * This function opens the file passed to it and assigns the fd to the pointer passed in
 */
int
opensql(const char *sqlfile, int *sqlfd) {
	int retc;
	retc = 0;

	if (dbg) {
		nxentr();
	}
	/* Quick sanity check */
	if ((sqlfile == NULL) || (sqlfd == NULL)) {
		nxerr("This should not be possible, NULL pointers presented");
		retc = -1;
	}

	/* Now that we've got some basic assurance that our pointers mean something, try opening the file */
	if ((*sqlfd = open(sqlfile,O_RDONLY|O_CLOEXEC)) == -1) {
		nxerr(strerror(errno));
		retc = -2;
	}

	if (dbg) { nxexit();}
	/* Nothing else necessary, simply return with the file descriptor assigned */
	return(retc);
}
