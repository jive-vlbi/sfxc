/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 2010 Joint Institute for VLBI in Europe
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xlrapi.h"

void
fatal(const char *emsg)
{
	fprintf(stderr, "fatal: %s: %s\n", emsg, strerror(errno));
	exit(EXIT_FAILURE);
}

int
print_labels(void)
{
	SSHANDLE xlrHandle;
	ULONG xlrError;
	char errString[XLR_ERROR_LENGTH];
	S_BANKSTATUS bankStatus;
	UINT bankID;
	char *p;

	if (XLROpen(1, &xlrHandle) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	if (XLRSetMode(xlrHandle, SS_MODE_SINGLE_CHANNEL) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	if (XLRClearChannels(xlrHandle) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	if (XLRSelectChannel(xlrHandle, 0) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	if (XLRBindOutputChannel(xlrHandle, 0) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	if (XLRSetBankMode(xlrHandle, SS_BANKMODE_NORMAL) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		fprintf(stderr, "%s\n", errString);
		XLRClose(xlrHandle);
		return -1;
	}

	for (bankID = BANK_A; bankID != BANK_INVALID; bankID++) {
		if (XLRGetBankStatus(xlrHandle, bankID, &bankStatus) != XLR_SUCCESS) {
			xlrError = XLRGetLastError();
			XLRGetErrorMessage(errString, xlrError);
			fprintf(stderr, "%s\n", errString);
			bankID = BANK_INVALID;
			break;
		}

		if (bankStatus.State == STATE_READY) {
			p = strchr(bankStatus.Label, '/');
			if (p)
				*p = 0;
			printf("%s\n", bankStatus.Label);
		}
	}

	XLRClose(xlrHandle);
	return 0;
}

int
main(void)
{
	struct sockaddr_in sin;
	int s;

	/*
	 * Check if Mark5A is running by connecting to localhost:2620.
	 * If that fails with ECONNREFUSED, we can be fairly certain
	 * no Mark5A-variant is running.
	 */
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1)
		fatal("socket");

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");
	sin.sin_port = htons(2620);

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) != -1)
		return (EXIT_FAILURE);
	if (errno != ECONNREFUSED)
		return (EXIT_FAILURE);

	print_labels();
	return (EXIT_SUCCESS);
}
