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

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Project specific headers */
#ifndef __EXILE_BUDGET_H
#include "budget.h"
#endif
#ifndef __BUDGETCONF_H
#include "budgetconf.h"
#endif

extern char *__progname;
extern char **environ;
extern bool dbg;

/* some basic implementations */
void
sparseconfig(const char *conffile) {
	register int cfd;
	ssize_t retc, written;
	char *defaults;
	cfd = 0;
	retc = written = 0;
	defaults = NULL;

	if ((cfd = open(conffile, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC, S_IRUSR|S_IWUSR)) == -1 ) {
		/* we have an invalid fd for this use */
		nxerr(strerror(errno));
		return;
	}
	/* this doesn't feel right at all, but clang was complaining about void* -> char* conversion */
	if ((defaults = (char *)calloc((size_t)PASS_MAX, sizeof(char))) == NULL) {
		nxerr(strerror(errno));
		close(cfd);
		return;
	}

	/* If we reached this point, we have a file descriptor and valid buffer */
	if ((retc = snprintf(defaults, (size_t)PASS_MAX, "database: %s/.local/.budget\npassword: \ndbhash: \nhashspec: SHA3-512\ncipherspec: ChaCha20\n", getenv("HOME"))) <= 0) {
		nxerr("Unable to write to buffer!");
		cfree(defaults,(size_t)PASS_MAX);
		return;
	}
	if ((written = write(cfd, defaults, (size_t)PASS_MAX)) != retc) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: Wrote less than expected, be sure to check %s/.config/budget.conf is correct!\nAttempting to continue with defaults...\n",
				__progname, __FILE__, __LINE__, __func__, getenv("HOME"));
	}
	/* ensure that the data has been committed to disk */
	fsync(cfd);
	close(cfd);
	cfree(defaults, (size_t)PASS_MAX);
	/* ensure that *defaults is set to NULL */
	defaults = NULL;
}

int 
parseconfig(int *fdptr, dbconfig *dbdata) {
	/* read from the given fd pointer and build the dbdata struct */
	int retc;
	retc = 0;
	if ((fdptr != NULL) && (dbdata != NULL) && (*fdptr > 2) ) {
		/* now we do things */
	}
	return(retc);
}

void
cfree(void *buf, size_t size) {
	register int i;
	i = WIPECNT;
	/* 
	 * while i goes to zero, 
	 * fill buffer with garbage
	 */
	while (i-->0) {
		arc4random_buf(buf, size);
	}
	free(buf);
	buf = NULL;
}

