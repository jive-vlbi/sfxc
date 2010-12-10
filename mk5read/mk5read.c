#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "xlrapi.h"

#include "mk5read.h"

#define MK5READ_SIZE	(256 * 4096)

/* Ulrich Drepper is a twat! */
#define strlcpy strncpy

#define min(a,b) ((a) < (b) ? (a) : (b))

volatile sig_atomic_t die;

void
fatal(const char *s)
{
	perror(s);
	exit(EXIT_FAILURE);
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

	buf = (ULONG *)malloc(MK5READ_SIZE);
	if (buf == NULL)
		return -1;

	while (!die) {
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
			printf("%s (%d)\n", errString, (int)xlrError);
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
	S_BANKSTATUS bankStatus;
	UINT bankID;
	char *p;

	if (XLROpen(1, xlrHandle) != XLR_SUCCESS)
		return -1;

	if (XLRSetMode(*xlrHandle, SS_MODE_SINGLE_CHANNEL) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRClearChannels(*xlrHandle) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSelectChannel(*xlrHandle, 0) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRBindOutputChannel(*xlrHandle, 0) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	if (XLRSetBankMode(*xlrHandle, SS_BANKMODE_NORMAL) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	for (bankID = BANK_A; bankID != BANK_INVALID; bankID++) {
		if (XLRGetBankStatus(*xlrHandle, bankID, &bankStatus) != XLR_SUCCESS) {
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

	if (XLRSelectBank(*xlrHandle, bankID) != XLR_SUCCESS) {
		XLRClose(*xlrHandle);
		return -1;
	}

	return 0;
}

void
handler(int sig)
{
	die = 1;
}

int
main(void)
{
 	SSHANDLE xlrHandle;
	ULONG xlrError;
	char errString[XLR_ERROR_LENGTH];

	struct sockaddr_un sun;
	socklen_t len;
	int s, fd;

	struct mk5read_msg msg;
	time_t t1, t2;
	int64_t xlen;

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

	while (!die) {
		len = sizeof(sun);
		fd = accept(s, (struct sockaddr *)&sun, &len);
		if (fd == -1)
			fatal("accept");

		if (read(fd, &msg, sizeof(msg)) != sizeof(msg))
			fatal("read");

		printf("%s:%llu\n", msg.vsn, (unsigned long long)msg.off);

		if (open_diskpack(msg.vsn, &xlrHandle) == -1) {
			xlrError = XLRGetLastError();
			XLRGetErrorMessage(errString, xlrError);
			printf("%s\n", errString);
			close(fd);
			continue;
		}

		t1 = time(NULL);
		xlen = stream_data(xlrHandle, msg.off, fd);
		t2 = time(NULL);
		if (xlen != -1 && t1 != t2)
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
