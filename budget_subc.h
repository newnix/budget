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
 * This file holds definitions for the subcommand parsing 
 * and execution down certain code paths, it's not necessarily
 * going to define all necessary subcommand functions, but it'll 
 * at least contain the parent functions
 */
#define __BUDGET_SUBS_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>

#ifndef __BUDGETCONF_H
#include "budgetconf.h"
#endif

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
 * This is the actual action taken on
 * the data in the database. 
 * There should be another enumerable
 * or a table of some sort to define 
 * if a user is creating groups or individual records.
 */
typedef enum _dbaction {
	create = 0, /* create a new record */
	update = 1, /* update an existing record */
	select = 2, /* pull info out of the database */
	delete = 3  /* delete a record */
} dbaction;

/* Read the string *input, and assign dbcmd->action accordingly */
int readaction(const char *input, dbcmd *dbcmd);
/* This reads the arguments after action and will try to parse out the user intent */
int parsecmd(const char **instr, dbcmd *dbcmd);
