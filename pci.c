/*-
 * Copyright (C) 2016 Chuck Tuffli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <sys/stat.h>

#include <pciaccess.h>
#include <libxo/xo.h>

extern void devlist(int argc, char *argv[]);
extern void devtree(int argc, char *argv[]);
extern void get_set(int argc, char *argv[]);
extern void reg_list(int argc, char *argv[]);

typedef void (*pci_fcn_t)(int argc, char *argv[]);

static struct pci_op {
	const char	*name;
	pci_fcn_t	fcn;
	const char	*usage;
} ops[] = {
	{"devlist", devlist, "       pci devlist [--libxo <args>] [-n] [-s selector]\n"},
	{"tree",    devtree, "       pci tree [--libxo <args>] [-n]\n"},
	{"set",     get_set, "       pci set -s <selector>\n"},
	{"get",     get_set, "       pci get -s <selector>\n"},
	{"reg",     reg_list,"       pci reg\n"},
	{NULL, NULL, NULL}
};

void
usage(void)
{
	struct pci_op *p = NULL;

	p = ops;
	fprintf(stderr, "usage:\n");
	while (p->name != NULL) {
		fprintf(stderr, "%s", p->usage);
		p++;
	}
}

int
main(int argc, char *argv[])
{
	char *op = "devlist";
	struct pci_op *p = NULL;

	argc = xo_parse_args(argc, argv);
	if (argc < 0)
		exit(EXIT_FAILURE);

	if (argc > 1) {
		op = argv[1];
	}

	if (pci_system_init()) {
		err(1, "Couldn't initialize PCI system");
	}

	p = ops;
	while (p->name != NULL) {
		if (strcmp(op, p->name) == 0) {
			p->fcn(argc - 1, &argv[1]);
			break;
		}
		p++;
	}

	if (p->name == NULL)
		usage();

	xo_finish();

	pci_system_cleanup();

	return EXIT_SUCCESS;
}
