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

#include <getopt.h>
#include <libxo/xo.h>
#include <pciaccess.h>

extern void create_db(void);
extern void destroy_db(void);
extern int lookup_db(uint16_t vid, uint16_t did, char **vname, char **dname);

static struct option opts[] = {
	{ "number", no_argument, NULL, 'n'},
	{ NULL, 0, NULL, 0 }
};

void
devlist(int argc, char *argv[])
{
	struct pci_device_iterator *iter = NULL;
	struct pci_device *pdev = NULL;
	int ch, verbose = 1;

	while ((ch = getopt_long(argc, argv, "n", opts, NULL)) != -1) {
		switch (ch) {
		case 'n':
			verbose = 0;
			break;
		default:
			return;
		}
	}

	if (verbose)
		create_db();

	iter = pci_slot_match_iterator_create(NULL);

	xo_open_list("device");

	while ((pdev = pci_device_next(iter)) != NULL) {
		xo_open_instance("device");
		xo_emit("{k:bdf/%04x:%02x:%02x.%u} ",
				pdev->domain, pdev->bus, pdev->dev, pdev->func);
		if (!verbose) {
			xo_emit("{k:vendorid/%04x}:{k:deviceid/%04x} {k:subvendorid/%04x}:{k:subdeviceid/%04x}\n",
					pdev->vendor_id, pdev->device_id,
					pdev->subvendor_id, pdev->subdevice_id);
		} else {
			char *vname = NULL, *dname = NULL;

			lookup_db(pdev->vendor_id, pdev->device_id, &vname, &dname);

			xo_emit("{k:vendorname} {k:devname}\n", vname, dname);
		}

		xo_close_instance("device");
	}

	xo_close_list("device");

	if (verbose)
		destroy_db();
}