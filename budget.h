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

#define __EXILE_BUDGET_H

/* 
 * These should probably be moved out of the headers unless strictly necessary for struct definitions
 */
#include <stdio.h>
#include <sqlite3.h>
#include <sys/types.h>

/* This file will only hold constants, prototypes, and custom types */
#ifndef PAGE_SIZE
/* 
 * This is going to be used to enable reading a full page out of the init file
 * and compiling it as SQLite bytecode directly, rather than trying to determine 
 * if we have a full statement available at a given time.
 */
#define PAGE_SIZE 4096
#endif
#ifndef PARAM_MAX 
#define PARAM_MAX 1024
#endif

/* Macros for runtime issues */
#define notimp(a) fprintf(stderr,"WRN: %s [%s:%u] %s: -%c is not implemented\n", __progname, __FILE__, __LINE__, __func__, a)
#define nxerr(message) fprintf(stderr,"ERR: %s [%s:%u] %s: %s\n", __progname,__FILE__,__LINE__,__func__,message)
#define nxwrn(message) fprintf(stderr,"WRN: %s [%s:%u] %s: %s\n", __progname,__FILE__,__LINE__,__func__,message)
#define nxdbg(message) fprintf(stderr,"DBG: %s [%s:%u] %s: %s\n", __progname,__FILE__,__LINE__,__func__,message)
#define nxinf(message) fprintf(stderr,"INF: %s [%s:%u] %s: %s\n", __progname,__FILE__,__LINE__,__func__,message)

/* Some default location macros */
#define DEFAULT_BUDGET_PARENTDIR getenv("HOME")
#define DEFAULT_BUDGET_DIR "/.local"
#define DEFAULT_BUDGET_DB ".budget"

/* 
 * Transaction ID size 
 */
#ifndef TID_LEN
#define TID_LEN 65
#endif

/* Custom types */
typedef enum __dbcmds {
	unknown = 0, /* default, still not sure what we're doing */
	insert = 1, /* create a new transaction record */
	query = 2, /* pull data out of the table */
	update = 3, /* update an existing entry */
	create = 4, /* create new xcats or xtypes */
	balance = 5, /* get the current estimated balance */
	show = 6 /* like query, but only accepts a category */
} dbaction;

/*
 * Unsure exactly what this should be at this point 
 */
typedef struct __cmdargs {
	dbaction action;
	/* this will default to a PAGE_SIZE buffer if small enough, else a mmap(2)'d file */
	unsigned char *dbsql;
} cmdargs;

/* 
 * This may be better as a simple array or
 * hash map, to allow potentially faster query 
 * construction based on the command line parsing
 * TODO: Revisit this topic
 */
typedef enum _xtype {
	expense = 0,
	deposit = 1,
	invoice = 2,
	investment = 3,
	salary = 4,
	adjustment = 5
} xtype;

/* 
 * Struct for actually holding database manipulation information
 */
typedef struct __dbcmd {
	sqlite3 *dbptr; /* handle for the database being used */
	unsigned char tid[TID_LEN]; /* 64 bits of data and a NULL terminator */
	dbaction action;
	xtype transtype;
	double amount;
} dbmcd;

/* Function Prototypes */
int cook(const char *dbname, const char *sqlfile, const char *cfgfile, const char *enckey, uint8_t flags);
int readconfig(const char *conffile);
int initialize(sqlite3 *dbptr, int *sqlfd);
int opensql(const char *sqlfile, int *sqlfd);
int mkexpense_category(cmdargs *dbdata, const char *category);
int insert_transaction(cmdargs *dbdata, const char *category, int cost);
/* this function may not be necessary any longer */
int buildcommand(const char **av, cmdargs *dbdata);
int connect(const char *dbname, sqlite3 *dbptr);
int decrypt(const char *dbname, const char *enckey);
