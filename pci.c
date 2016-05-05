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
#include <sys/queue.h>
#include <libxo/xo.h>

extern void devlist(int argc, char *argv[]);
extern void devtree(int argc, char *argv[]);

typedef void (*pci_fcn_t)(int argc, char *argv[]);

static struct pci_op {
	const char	*name;
	pci_fcn_t	fcn;
	const char	*usage;
} ops[] = {
	{"devlist", devlist, "       pci devlist [--libxo <args>] [-n]\n"},
	{"tree",    devtree, "       pci tree [--libxo <args>] [-n]\n"},
	{NULL, NULL, NULL}
};

static void
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

typedef struct pcidb_dev_s {
	uint16_t	id;
	char		*str;
	TAILQ_ENTRY(pcidb_dev_s) link;
} pcidb_dev_t;

typedef struct pcidb_vendor_s {
	uint16_t	id;
	char		*str;
	TAILQ_HEAD(,pcidb_dev_s) dlist;
	TAILQ_ENTRY(pcidb_vendor_s) link;
} pcidb_vendor_t;

static TAILQ_HEAD(,pcidb_vendor_s) vlist;


static const char *dblist[] = {
#ifdef __linux__
	"/usr/share/misc/pci.ids",		/* Linux default */
#endif
#ifdef __FreeBSD__
	"/usr/local/share/pciids/pci.ids",	/* FreeBSD ports */
	"/usr/share/misc/pci_vendors",		/* FreeBSD base */
#endif
	NULL
};

void
create_db(void)
{
	FILE	*ids;
	size_t	line = 0;
	char	b[1024], desc[1024];
	uint32_t vid, did;
	pcidb_vendor_t	*curvend = NULL;
	pcidb_dev_t *curdev = NULL;
	const char *dbfile = NULL;
	struct stat s;

	TAILQ_INIT(&vlist);

	if ((dbfile = getenv("PCI_ID_DATABASE")) == NULL) {
		uint32_t i = 0;

		while (dblist[i]) {
			dbfile = dblist[i];

			if (stat(dbfile, &s) != -1) {
				break;
			}
			i++;
		}
	}

	if ((ids = fopen(dbfile, "r")) == NULL) {
		printf("can't open %s\n", dbfile);
		return;
	}

	while (fgets(b, sizeof(b), ids) != NULL) {
		line++;

		if (b[0] == '#')
			continue;

		/* skip decode of sub-vendor and device ids */
		if (b[0] == '\t' && b[1] == '\t')
			continue;

		/* Vendor ID entry */
		if (b[0] != '\t') {
			if (sscanf(b, "%04x %[^\n]", &vid, desc) == 2) {
				if (vid == 0 || strlen(desc) < 1)
					continue;


				curvend = malloc(sizeof(pcidb_vendor_t));
				if (curvend == NULL) {
					warn("malloc pcidb_vendor_t");
					break;
				}

				curvend->id = vid;
				curvend->str = strdup(desc);
				TAILQ_INIT(&curvend->dlist);

				TAILQ_INSERT_TAIL(&vlist, curvend, link);
			}
		} else {
			if (sscanf(b+1, "%04x %[^\n]", &did, desc) == 2) {
				if (did == 0 || strlen(desc) < 1)
					continue;

				if (curvend == NULL) {
					warnx("No vendor line %zd", line);
					continue;
				}

				curdev = malloc(sizeof(pcidb_dev_t));
				if (curdev == NULL) {
					warn("malloc pcidb_dev_t");
					break;
				}

				curdev->id = did;
				curdev->str = strdup(desc);

				TAILQ_INSERT_TAIL(&curvend->dlist, curdev, link);
			}
		}
	}

	fclose(ids);
}

int
lookup_db(uint16_t vid, uint16_t did, char **vname, char **dname)
{
	pcidb_vendor_t *v = NULL;

	TAILQ_FOREACH(v, &vlist, link) {
		if (v->id == vid) {
			pcidb_dev_t *d = NULL;

			TAILQ_FOREACH(d, &v->dlist, link) {
				if (d->id == did) {
					*vname = v->str;
					*dname = d->str;

					break;
				}
			}
			break;
		}
	}

	return 0;
}

void
destroy_db(void)
{
	pcidb_vendor_t *v, *vtmp;
	pcidb_dev_t *d, *dtmp;

	v = TAILQ_FIRST(&vlist);
	while (v != NULL) {
		vtmp = TAILQ_NEXT(v, link);

		d = TAILQ_FIRST(&v->dlist);
		while (d != NULL) {
			dtmp = TAILQ_NEXT(d, link);

			free(d->str);
			free(d);

			d = dtmp;
		}

		free(v->str);
		free(v);

		v = vtmp;
	}
}
