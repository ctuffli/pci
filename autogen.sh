#!/bin/sh
#
# Copyright (C) 2016 Chuck Tuffli
# All rights reserved.
# This SOFTWARE is licensed under the LICENSE provided in the
# Copyright file. By downloading, installing, copying, or otherwise
# using the SOFTWARE, you agree to be bound by the terms of that
# LICENSE.
if [ ! -f configure ]; then
	echo "Generating configure with " `autoreconf --version | head -1`

	autoreconf -fvi

	if [ ! -f configure ]; then
		echo "Unable to create configure script"
		exit 1
	fi
fi

exit 0

