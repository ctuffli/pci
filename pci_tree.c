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
#include <sys/queue.h>

extern const char *pci_device_get_class_name( const struct pci_device * );

static struct option opts[] = {
	{ "number", no_argument, NULL, 'n'},
	{ NULL, 0, NULL, 0 }
};

STAILQ_HEAD(bus_list_s, bus_s) hostbus = STAILQ_HEAD_INITIALIZER(hostbus);
struct bus_list_s buses = STAILQ_HEAD_INITIALIZER(buses);

struct bus_s {
	uint8_t bus;
	struct pci_device *parent;
	STAILQ_ENTRY(bus_s)	entries;

	STAILQ_HEAD(pdev_list_s, pdev_s) devices;
};

struct pdev_s {
	struct pci_device *dev;
	STAILQ_ENTRY(pdev_s)	entries;
};

static struct bus_s *get_bus(struct bus_list_s *bl, uint8_t id);
static struct bus_s *add_bus(struct bus_list_s *bl, uint8_t id, struct pci_device *parent);
static struct pdev_s *add_device(struct bus_s *bus, struct pci_device *pdev);
static void print_bus_tree(struct bus_s *b, uint32_t depth, int verbose);
static void free_bus_list(struct bus_list_s *bl);

void
devtree(int argc, char *argv[])
{
	struct pci_device_iterator *iter;
	struct pci_device *pdev;
	struct bus_s *b;
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

	iter = pci_slot_match_iterator_create(NULL);

	/*
	 * Loop through all devices to create the PCI hierarchy
	 */
	while ((pdev = pci_device_next(iter)) != NULL) {
		b = get_bus(&buses, pdev->bus);
		if (b == NULL) {
			add_bus(&hostbus, pdev->bus, NULL);
			b = add_bus(&buses, pdev->bus, NULL);
		}

		const struct pci_bridge_info *binfo = NULL;

		binfo = pci_device_get_bridge_info(pdev);
		if ((binfo != NULL) && (binfo->secondary_bus != 0)) {
			struct bus_s *sb;
			uint8_t i;

			for (i = binfo->secondary_bus; i <= binfo->subordinate_bus; i++) {
				sb = get_bus(&buses, i);
				if (sb == NULL) {
					add_bus(&buses, i, pdev);
				} else {
					/* if the bus already exists, update the parent */
					sb->parent = pdev;
				}
			}
		}

		add_device(b, pdev);
	}

	/*
	 * Print the bus tree
	 */
	struct bus_s *hb = NULL;

	xo_attr("id", "%04x", 0);
	xo_open_container("domain");

	xo_open_list("bus");

	STAILQ_FOREACH(hb, &hostbus, entries) {
		print_bus_tree(get_bus(&buses, hb->bus), 1, verbose);
	}

	xo_close_list("bus");

	xo_close_container("domain");

	free_bus_list(&buses);
}

static struct bus_s *
get_bus(struct bus_list_s *bl, uint8_t id)
{
	struct bus_s *b = NULL;

	STAILQ_FOREACH(b, bl, entries) {
		if (b->bus == id) {
			return b;
		}
	}

	return NULL;
}

static struct bus_s *
add_bus(struct bus_list_s *bl, uint8_t id, struct pci_device *parent)
{
	struct bus_s *b = NULL;

	b = malloc(sizeof(struct bus_s));
	if (b != NULL) {
		b->bus = id;
		b->parent = parent;
		STAILQ_INIT(&b->devices);

		STAILQ_INSERT_TAIL(bl, b, entries);
	}

	return b;
}

static struct pdev_s *
add_device(struct bus_s *bus, struct pci_device *device)
{
	struct pdev_s *p = NULL;

	p = malloc(sizeof(struct pdev_s));
	if (p != NULL) {
		p->dev = device;

		STAILQ_INSERT_TAIL(&bus->devices, p, entries);
	}

	return p;
}

static void
print_bus_tree(struct bus_s *b, uint32_t depth, int verbose)
{
	struct pdev_s *d = NULL;

	xo_attr("id", "%04x", b->bus);
	xo_open_instance("bus");

	xo_emit("{P:/%*s}{L:/%04x:%02x} =>\n",
			(depth - 1) * 4, "",
			b->parent ? b->parent->domain : 0, b->bus);

	xo_open_list("device");

	STAILQ_FOREACH(d, &b->devices, entries) {
		struct pci_device *pdev = d->dev;

		xo_open_instance("device");

		xo_emit("{P:/%*s}{k:bdf/%04x:%02x:%02x.%u} ",
				depth * 4, "",
				pdev->domain, pdev->bus, pdev->dev, pdev->func);

		if (!verbose) {
			xo_emit("{k:vendorid/%04x}:{k:deviceid/%04x} {k:subvendorid/%04x}:{k:subdeviceid/%04x}\n",
					pdev->vendor_id, pdev->device_id,
					pdev->subvendor_id, pdev->subdevice_id);
		} else {
			const char *cname = NULL, *vname = NULL, *dname = NULL;

			cname = pci_device_get_class_name(pdev);
			vname = pci_device_get_vendor_name(pdev);
			dname = pci_device_get_device_name(pdev);

			xo_emit("{k:classname} {k:vendorname} {k:devname}\n", cname, vname, dname);
		}

		const struct pci_bridge_info *binfo = NULL;

		binfo = pci_device_get_bridge_info(pdev);
		if ((binfo != NULL) && (binfo->secondary_bus > 0)) {
			struct bus_s *sb = get_bus(&buses, binfo->secondary_bus);

			if (sb != NULL) {
				xo_open_list("bus");
				print_bus_tree(sb, depth + 1, verbose);
				xo_close_list("bus");
			}
		}

		xo_close_instance("device");
	}

	xo_close_list("device");

	xo_close_instance("bus");
}

static void
free_bus_list(struct bus_list_s *bl)
{
	struct bus_s *b, *bn;

	b = STAILQ_FIRST(bl);
	while (b != NULL) {
		bn = STAILQ_NEXT(b, entries);

		if (!STAILQ_EMPTY(&b->devices)) {
			struct pdev_s *d, *dn;

			d = STAILQ_FIRST(&b->devices);
			while (d != NULL) {
				dn = STAILQ_NEXT(d, entries);
				free(d);
				d = dn;
			}
		}

		free(b);
		b = bn;
	}
	STAILQ_INIT(bl);
}
