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
 * This file will hold some of the config file related functions
 */
/* necessary headers */
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 
 * Set up some size constraints
 */
#define __BUDGETCONF_H
#ifndef PATH_MAX
#define PATH_MAX 512
#endif
#ifndef PASS_MAX
#define PASS_MAX 1024
#endif
#ifndef HASHLEN
#define HASHLEN 512
#endif
#ifndef WIPECNT
#define WIPECNT 8
#endif

/* 
 * necessary externs
 */
extern char *__progname;
extern char **environ;

/* 
 * Now specify acceptable encryption options 
 */
typedef enum __hashspec {
	none = 0,
	sha256 = 1,
	sha512 = 2,
	whirlpool = 4,
	shake256 = 8,
	blake2b512 = 16,
	sha512256 = 32,
	sha385 = 64,
	sha3512 = 128,
	sha3256 = 256
} hashspec;

typedef enum __cipherspec {
	aes256_cbc = 0,
	aes256_ctr = 1,
	serpent256_cbc = 2,
	serpent256_cmc = 4,
	twofish_cbc = 8,
	chacha20poly1305 = 16
} cipherspec;

/* 
 * Ensure we have a dbconfig struct available for manipulation
 */
typedef struct __dbconf {
	char dbname[PATH_MAX];
	/* these members may not actually be used until later vorsions */
	char password[PASS_MAX];
	char dbhash[HASHLEN];
	hashspec hash;
	cipherspec cipher;
	/* may add a SQLite3 database pointer in later revisions */
} dbconfig;

/* Create a basic config file if one isn't found */
void cfree(void *buf, size_t size);
void sparseconfig(const char *conffile);
void checkparam(const char *confline, dbconfig *confdata);
int parseconfig(int *fdptr, dbconfig *dbdata);

/* some basic implementations */
void
sparseconfig(const char *conffile) {
	register int cfd;
	ssize_t retc, written;
	char *defaults;
	cfd = 0;
	retc = written = 0;
	defaults = NULL;

	if ((cfd = open(conffile, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR)) < 1 ) {
		/* we have an invalid fd for this use */
		fprintf(stderr, "ERR: %s [%s:%u] %s: %s!\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		return;
	}
	/* this doesn't feel right at all, but clang was complaining about void* -> char* conversion */
	if ((defaults = (char *)calloc((size_t)PASS_MAX, sizeof(char))) == NULL) {
		fprintf(stderr, "%s [%s:%u] %s: %s!\n",
				__progname, __FILE__, __LINE__, __func__, strerror(errno));
		close(cfd);
		return;
	}

	/* If we reached this point, we have a file descriptor and valid buffer */
	if ((retc = snprintf(defaults, (size_t)PASS_MAX, "database: %s/.local/.budget\npassword: \ndbhash: \nhashspec: SHA3-512\ncipherspec: ChaCha20\n", getenv("HOME"))) <= 0) {
		fprintf(stderr, "ERR: %s [%s:%u] %s: Unable to write default config data to buffer at %p!\n", 
				__progname, __FILE__, __LINE__, __func__, (void*)defaults);
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
