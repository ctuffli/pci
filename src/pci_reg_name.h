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

struct reg_name {
	const char *name;
	const uint32_t offset;
	const uint32_t width;
};

static struct reg_name reg_name_map[] = {
	/* Type 0/1 Common Configuration Space */
	{ "VENDOR",	 0, 2 },
	{ "DEVICE",	 2, 2 },
	{ "COMMAND",	 4, 2 },
	{ "STATUS",	 6, 2 },
	{ "REVISION",	 8, 1 },
	{ "CLASS_PROG",  9, 1 },
	{ "CLASS_DEV",  10, 1 },
	{ "CACHE_LINE", 12, 1 },
	{ "PRIMARY_LATENCY", 13, 1 },
	{ "HEADER_TYPE", 14, 1 },
	{ "BIST",       15, 1 },
	{ "BAR_0",	16, 4 },	/* technically not common but are shared */
	{ "BAR_1",	20, 4 },	/* technically not common but are shared */
	{ "EROM_BAR",   48, 4 },	/* technically not common but are shared */
	{ "CAPABILITIES", 52, 1 },
	{ "INTERRUPT_LINE", 56, 1 },
	{ "INTERRUPT_PIN", 57, 1 },

	/* Type 0 Configuration Space */
	{ "BAR_2",	24, 4 },
	{ "BAR_3",	28, 4 },
	{ "BAR_4",	32, 4 },
	{ "BAR_5",	36, 4 },
	{ "CARDBUS_CIS", 40, 4 },
	{ "SUBSYSTEM_VENDOR", 44, 2 },
	{ "SUBSYSTEM_DEVICE", 46, 2 },
	{ "MIN_GNT",    58, 1 },
	{ "MAX_LAT",    59, 1 },

	/* Type 1 Configuration Space */
	{ "PRIMARY_BUS", 24, 1 },
	{ "SECONDARY_BUS", 25, 1 },
	{ "SUBORDINATE_BUS", 26, 1 },
	{ "SECONDARY_LATENCY", 27, 1 },
	{ "IO_BASE",    28, 1 },
	{ "IO_LIMIT",   29, 1 },
	{ "SECONDARY_STATUS", 30, 2 },
	{ "MEM_BASE",   32, 2 },
	{ "MEM_LIMIT",  34, 2 },
	{ "PREFETCH_BASE", 36, 2 },
	{ "PREFETCH_LIMIT", 38, 2 },
	{ "PREFETCH_BASE_UPPER", 40, 4 },
	{ "PREFETCH_LIMIT_UPPER", 44, 4 },
	{ "IO_BASE_UPPER", 48, 2 },
	{ "IO_LIMIT_UPPER", 50, 2 },
	{ "BRIDGE_CONTROL", 58, 2 },
	{ NULL, 0, 0 }
};

