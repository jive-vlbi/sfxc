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
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "xlrapi.h"

#include "mk5read.h"

#define MK5READ_SIZE	(256 * 4096)

/* Ulrich Drepper is a twat! */
#define strlcpy strncpy

#define min(a,b) ((a) < (b) ? (a) : (b))

int debug;

void	logit(int, const char *, ...);
void	vlog(int, const char *, va_list);

void
log_init(int n_debug)
{
	extern char *__progname;

	debug = n_debug;

	if (!debug)
		openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

	tzset();
}

void
fatal(const char *emsg)
{
	if (errno)
		logit(LOG_CRIT, "fatal: %s: %s\n", emsg, strerror(errno));
	else
		logit(LOG_CRIT, "fatal: %s\n", emsg);
}

void
logit(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vlog(pri, fmt, ap);
	va_end(ap);
}

void
vlog(int pri, const char *fmt, va_list ap)
{
	char *nfmt;

	if (debug) {
		/* best effort in out of mem situations */
		if (asprintf(&nfmt, "%s\n", fmt) == -1) {
			vfprintf(stderr, fmt, ap);
			fprintf(stderr, "\n");
		} else {
			vfprintf(stderr, nfmt, ap);
			free(nfmt);
		}
		fflush(stderr);
	} else
		vsyslog(pri, fmt, ap);
}

int64_t
stream_data(SSHANDLE xlrHandle, uint64_t off, int fd)
{
	S_READDESC readDesc;
	ULONG xlrError;
	char errString[XLR_ERROR_LENGTH];
	ULONG *buf;
	DWORDLONG recordingLength;
	uint64_t len = 0, size;
	ssize_t nbytes;

	recordingLength = XLRGetLength(xlrHandle);
	if (recordingLength == 0) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		return -1;
	}

	buf = (ULONG *)malloc(MK5READ_SIZE);
	if (buf == NULL)
		return -1;

	for (;;) {
		if (off >= recordingLength)
			break;
		size = min(recordingLength - off, MK5READ_SIZE);
		readDesc.AddrHi = off >> 32;
		readDesc.AddrLo = off & 0xffffffff;
		readDesc.XferLength = size;
		readDesc.BufferAddr = buf;
		if (XLRRead(xlrHandle, &readDesc) != XLR_SUCCESS) {
			xlrError = XLRGetLastError();
			XLRGetErrorMessage(errString, xlrError);
			logit(LOG_CRIT, "%s", errString);

			/*
			 * Close the filedescriptor early, such that
			 * the correlator doesn't wait for us.
			 */
			close(fd);

			/*
			 * Whack the StreamStor hardware over the
			 * head.  We do the XLRClose()/XLROpen() dance
			 * to force reloading of the firmware and
			 * reinitializion of the hardware, such that
			 * it is ready for the next request.
			 */
			XLRReset(xlrHandle);
			XLRClose(xlrHandle);
			XLROpen(1, &xlrHandle);
			break;
		}

		nbytes = write(fd, buf, size);
		off += nbytes;
		len += nbytes;
		if (nbytes != (ssize_t)size)
			break;
	}

	return len;
}

int
open_diskpack(const char *vsn, SSHANDLE *xlrHandle)
{
	ULONG xlrError;
	char errString[XLR_ERROR_LENGTH];
	S_BANKSTATUS bankStatus;
	UINT bankID;
	char *p;

	if (XLROpen(1, xlrHandle) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSetMode(*xlrHandle, SS_MODE_SINGLE_CHANNEL) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRClearChannels(*xlrHandle) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSelectChannel(*xlrHandle, 0) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRBindOutputChannel(*xlrHandle, 0) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSetBankMode(*xlrHandle, SS_BANKMODE_NORMAL) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSetFillData(*xlrHandle, 0x11223344) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	/*
	 * 
	 */
	for (bankID = BANK_A; bankID != BANK_INVALID; bankID++) {
		if (XLRGetBankStatus(*xlrHandle, bankID, &bankStatus) != XLR_SUCCESS) {
			xlrError = XLRGetLastError();
			XLRGetErrorMessage(errString, xlrError);
			logit(LOG_CRIT, "%s", errString);
			bankID = BANK_INVALID;
			break;
		}

		if (bankStatus.State == STATE_READY) {
			p = strchr(bankStatus.Label, '/');
			if (p)
				*p = 0;
			if (strcmp(vsn, bankStatus.Label) == 0)
				break;
		}
	}

	if (bankID == BANK_INVALID)
		return -1;

	if (bankStatus.MediaStatus == MEDIASTATUS_FAULTED) {
		if (XLRSetOption(*xlrHandle, SS_OPT_SKIPCHECKDIR) != XLR_SUCCESS) {
			xlrError = XLRGetLastError();
			XLRGetErrorMessage(errString, xlrError);
			logit(LOG_CRIT, "%s", errString);
			XLRClose(*xlrHandle);
			return -1;
		}
	}

	if (XLRSelectBank(*xlrHandle, bankID) != XLR_SUCCESS) {
		xlrError = XLRGetLastError();
		XLRGetErrorMessage(errString, xlrError);
		logit(LOG_CRIT, "%s", errString);
		XLRClose(*xlrHandle);
		return -1;
	}

	return 0;
}

void
handler(int sig)
{
	unlink(MK5READ_SOCKET);
	_exit(EXIT_FAILURE);
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d]\n", __progname);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
 	SSHANDLE xlrHandle;
	struct sockaddr_un sun;
	socklen_t len;
	int s, fd;
	struct mk5read_msg msg;
	time_t t1, t2;
	int64_t xlen;
	int debug = 0;
	int ch;

	log_init(1);

	while ((ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
		case 'd':
			debug = 1;
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

	if (!debug)
		if (daemon(0, 0))
			fatal("daemon");

	log_init(debug);

	signal(SIGTERM, handler);
	signal(SIGPIPE, SIG_IGN);

	s = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (s == -1)
		fatal("socket");

	sun.sun_family = AF_LOCAL;
	strlcpy(sun.sun_path, MK5READ_SOCKET, sizeof(sun.sun_path));
	unlink(MK5READ_SOCKET);
	if (bind(s, (struct sockaddr *)&sun, sizeof(sun)) == -1)
		fatal("bind");

	if (listen(s, 1) == -1)
		fatal("listen");

	for (;;) {
		len = sizeof(sun);
		fd = accept(s, (struct sockaddr *)&sun, &len);
		if (fd == -1)
			fatal("accept");

		if (read(fd, &msg, sizeof(msg)) != sizeof(msg))
			fatal("read");

		if (debug)
			printf("%s:%llu\n", msg.vsn,
			    (unsigned long long)msg.off);

		if (open_diskpack(msg.vsn, &xlrHandle) == -1) {
			close(fd);
			continue;
		}

		t1 = time(NULL);
		xlen = stream_data(xlrHandle, msg.off, fd);
		t2 = time(NULL);
		if (debug && xlen != -1 && t1 != t2)
			printf("%lld bytes in %d seconds: %f Mbit/s\n",
			       (long long)xlen, (int)(t2 - t1),
			       (double)xlen * 8 * 1e-6 / (t2 - t1));

		XLRClose(xlrHandle);

		close(fd);
	}

	if (unlink(MK5READ_SOCKET) == -1)
		fatal("unlink");
	return (EXIT_SUCCESS);
}
