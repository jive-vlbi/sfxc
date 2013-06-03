/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 2010, 2011 Joint Institute for VLBI in Europe
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
#include <unistd.h>

#include "xlrapi.h"

void
fatal(const char *emsg)
{
	fprintf(stderr, "fatal: %s: %s\n", emsg, strerror(errno));
	exit(EXIT_FAILURE);
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-f]\n", __progname);
	exit(EXIT_FAILURE);
}

int
print_labels(void)
{
	SSHANDLE xlrHandle;
	ULONG xlrError;
	char errString[XLR_ERROR_LENGTH];
	S_BANKSTATUS bankStatus;
	UINT32 bankID;
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

void
print_mk5read_reply(char reply[], const int size){
  int i,j;
  char vsn[2][size];
  sscanf(reply, "!bank_set? 0 : A : %s : B : %s", vsn[0], vsn[1]);
  for(i=0;i<2;i++){
    j = 0;
    while(vsn[i][j] != '\0'){
      if ((vsn[i][j] == '/') || (vsn[i][j] == ':') || (vsn[i][j] == ';')){
        vsn[i][j] = '\0';
        break;
      }
      j++;
    }
    printf("%s\n", vsn[i]);
  }
}

int
main(int argc, char **argv)
{
	struct sockaddr_in sin;
        char request[1024], reply[1024];
	int ch, force = 0;
	int s, nbytes;

	while ((ch = getopt(argc, argv, "f")) != -1) {
		switch (ch) {
		case 'f':
			force = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	if (force) {
	        print_labels();
        }else{
		/*
		 * Check if Mark5A is running by connecting to
		 * localhost:2620.  If that fails with ECONNREFUSED,
		 * we can be fairly certain no Mark5A-variant is
		 * running.
		 */
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s == -1)
			fatal("socket");

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr("127.0.0.1");
		sin.sin_port = htons(2620);

		if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == 0){
                     strcpy(request, "bank_set?");
                     write(s, request, sizeof(request));
                     nbytes = read(s, reply, sizeof(reply));
                     if(nbytes <= 0)
                         return(EXIT_FAILURE);
                     print_mk5read_reply(reply, sizeof(reply));
                }else{
			print_labels();
                }
	}

	return (EXIT_SUCCESS);
}
