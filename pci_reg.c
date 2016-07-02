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

#include <ctype.h>
#include <string.h>
#include <err.h>
#include <getopt.h>
#include <libxo/xo.h>
#include <pciaccess.h>

#include "pci_reg_name.h"

#define MAX_STACK	4

extern void usage(void);
extern int isdigit(int);

static struct option opts[] = {
	{ "selector", required_argument, NULL, 's'},
	{ NULL, 0, NULL, 0 }
};

/**
 * Allowable selector combinations:
 *   domain:bus:device.function
 *   bus:device.function
 *   bus:device
 *   device
 * 
 * Use 'x' or '*' to wildcard a selector value. E.g.
 *    "5:x" matches all devices on bus 5
 */
struct pci_slot_match *
parse_selector(const char *s)
{
	struct pci_slot_match *pmatch = NULL;
	uint32_t sel_stack[MAX_STACK];
	char *s_end = NULL;
	uint32_t depth = 0;

	if (s == NULL) {
		return NULL;
	}

	/*
	 * Scan the input string looking for numbers, wild card characters,
	 * and separators.
	 */
	while ((depth < MAX_STACK)) {
		if (isdigit(*s)) {
			sel_stack[depth++] = strtoul(s, &s_end, 0);
			s = s_end;
		} else if ((*s == '*') || (*s == 'x')) {
			sel_stack[depth++] = PCI_MATCH_ANY;
			s++;
		} else {
			break;
		}

		if (*s == ':' || *s == '.') {
			s++;
		}
	}

	/*
	 * Convert the elements of the selector into a PCI BDF
	 */
	if (depth == 0) {
		printf("Can't parse selector string\n");
	} else {
		pmatch = malloc(sizeof(struct pci_slot_match));
		if (pmatch) {
			pmatch->domain = PCI_MATCH_ANY;
			pmatch->bus = PCI_MATCH_ANY;
			pmatch->dev = PCI_MATCH_ANY;
			pmatch->func = PCI_MATCH_ANY;
			pmatch->match_data = 0;

			if (depth > 2)
				pmatch->func = sel_stack[--depth];
			if (depth > 0)
				pmatch->dev = sel_stack[--depth];
			if (depth > 0)
				pmatch->bus = sel_stack[--depth];
			if (depth > 0)
				pmatch->domain = sel_stack[--depth];
		}
	}

	return pmatch;
}

/**
 * Parse the given offset, be it a number (0, 0x0) or name ("VENDOR")
 */
static int32_t
parse_offset(const char *o, uint32_t *offset, uint32_t *width)
{
	char *o_end = NULL;
	uint32_t off = 0;
	uint32_t w = 4;

	if ((o == NULL) || (offset == NULL) || (width == NULL)) {
		return EINVAL;
	}

	off = strtoul(o, &o_end, 0);

	if (o_end == o) { /* String isn't a number */
		struct reg_name *r = reg_name_map;

		while (r->name != NULL) {
			if (strcmp(o, r->name) == 0) {
				off = r->offset;
				w   = r->width;
				break;
			}
			r++;
		}

		if (r->name == NULL) {
			printf("Unrecognized offset name '%s'\n", o);
			return -1;
		}
	} else if (*o_end == '.') { /* String includes a width */
		o_end++;
		if (*o_end != 0) {
			switch (*o_end) {
			case 'b': w = 1; break;
			case 'h': w = 2; break;
			case 'w': w = 4; break;
			default:  w = 4; break;
			}
		}
	}

	*offset = off;
	*width = w;

	return 0;
}

/**
 * Write a configuration register offset
 *
 * Writes value pointed to by v to offset off
 */
static int32_t
write_cfg(struct pci_device *pdev, uint32_t off, void *v, uint32_t width)
{

	if (v == NULL) {
		return EINVAL;
	}

	switch (width) {
	case 1:
		return pci_device_cfg_write_u8(pdev, *((uint8_t *)v), off); 
	case 2:
		return pci_device_cfg_write_u16(pdev, *((uint16_t *)v), off);
	case 4:
		return pci_device_cfg_write_u32(pdev, *((uint32_t *)v), off);
	default:
		return ENODEV;
	}
}

/**
 * Read a configuration register offset
 *
 * Reads value at offset off into v
 */
static int32_t
read_cfg(struct pci_device *pdev, uint32_t off, void *v, uint32_t width)
{

	switch (width) {
	case 1:
		return pci_device_cfg_read_u8(pdev, v, off);
	case 2:
		return pci_device_cfg_read_u16(pdev, v, off);
	case 4:
		return pci_device_cfg_read_u32(pdev, v, off);
	default:
		return ENODEV;
	}
}

/**
 * Get or set a PCI configuration register
 */
void
get_set(int argc, char *argv[])
{
	int ch;
	const char *sel_str = NULL;

	while ((ch = getopt_long(argc, argv, "s:", opts, NULL)) != -1) {
		switch (ch) {
		case 's':
			sel_str = optarg;
			break;
		default:
			return;
		}
	}

	argc -= optind;
	argv += optind;

	if (sel_str != NULL) {
		struct pci_slot_match *pmatch = NULL;
		uint32_t off = UINT32_MAX;
		uint32_t width = UINT32_MAX;
		uint32_t val = 0;
		char *val_end = NULL;
		int32_t (*op)(struct pci_device *, uint32_t, void *, uint32_t) = read_cfg;

		if (argc < 1) {
			printf("Missing offset\n");
			usage();
			return;
		}

		parse_offset(argv[0], &off, &width);

		if (argc > 1) {
			op = write_cfg;
			val = strtoul(argv[1], &val_end, 0);
		}

		pmatch = parse_selector(sel_str);
		if (pmatch) {
			struct pci_device_iterator *iter = NULL;
			struct pci_device *pdev = NULL;
#if 0
			printf("Trying to match %04x:%02x:%02x.%x\n",
					pmatch->domain, pmatch->bus, pmatch->dev, pmatch->func);
#endif
			iter = pci_slot_match_iterator_create(pmatch);
			while (NULL != (pdev = pci_device_next(iter))) {
				printf("%s %04x:%02x:%02x.%u %x ",
						op == write_cfg ? "set" : "get",
						pdev->domain, pdev->bus, pdev->dev, pdev->func,
						off);

				if (op(pdev, off, &val, width))
					err(1, op == write_cfg ? "set" : "get");
				else
					printf("0x%0*x\n", width * 2, val); 

				val = 0;
			}

			free(iter);
			free(pmatch);
		} else {
			printf("Bad selector format\n");
			usage();
		}
	} else {
		printf("Missing selector\n");
		usage();
	}
}

/**
 * List the symbolic register names and their width / offset
 */
void
reg_list(int argc, char *argv[])
{
	struct reg_name *r = reg_name_map;

	printf("%20s %6s %s\n", "Name", "Offset", "Width");
	while (r->name != NULL) {
		printf("%20s %#6x %5u\n", r->name, r->offset, r->width);
		r++;
	}
}

