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

extern const char *pci_device_get_class_name( const struct pci_device * );

extern struct pci_slot_match *parse_selector(const char *s);

static struct option opts[] = {
	{ "number", no_argument, NULL, 'n'},
	{ "selector", required_argument, NULL, 's'},
	{ NULL, 0, NULL, 0 }
};

void
devlist(int argc, char *argv[])
{
	struct pci_device_iterator *iter = NULL;
	struct pci_device *pdev = NULL;
	struct pci_slot_match *pmatch = NULL;
	int ch, verbose = 1;
	const char *sel_str = NULL;

	while ((ch = getopt_long(argc, argv, "ns:", opts, NULL)) != -1) {
		switch (ch) {
		case 'n':
			verbose = 0;
			break;
		case 's':
			sel_str = optarg;
			break;
		default:
			return;
		}
	}

	if (sel_str != NULL) {
		pmatch = parse_selector(sel_str);
#if 0
		if (pmatch)
			printf("Trying to match %04x:%02x:%02x.%x\n",
					pmatch->domain, pmatch->bus, pmatch->dev, pmatch->func);
#endif
	}

	iter = pci_slot_match_iterator_create(pmatch);

	xo_open_list("device");

	while ((pdev = pci_device_next(iter)) != NULL) {
		xo_open_instance("device");
		xo_emit("{k:bdf/%04x:%02x:%02x.%u} ",
				pdev->domain, pdev->bus, pdev->dev, pdev->func);
		if (verbose) {
			const char *cname = NULL, *vname = NULL, *dname = NULL;

			cname = pci_device_get_class_name(pdev);
			vname = pci_device_get_vendor_name(pdev);
			dname = pci_device_get_device_name(pdev);

			xo_emit("{k:classname}: {k:vendorname} {k:devname}\n", cname, vname, dname);
		} else {
			xo_emit("{k:vendorid/%04x}:{k:deviceid/%04x} {k:subvendorid/%04x}:{k:subdeviceid/%04x} {k:class/%06x}\n",
					pdev->vendor_id, pdev->device_id,
					pdev->subvendor_id, pdev->subdevice_id,
					pdev->device_class);
		}

		xo_close_instance("device");
	}

	xo_close_list("device");
}
