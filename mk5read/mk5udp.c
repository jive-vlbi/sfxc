/*
 * Copyright (c) 2011 Joint Institute for VLBI in Europe
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
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mk5read.h"

/* Ulrich Drepper is a twat! */
#define strlcpy strncpy

int debug = 0;
int port = 2630;

void
fatal(const char *emsg)
{
	fprintf(stderr, "fatal: %s: %s\n", emsg, strerror(errno));
	exit(EXIT_FAILURE);
}

uint16_t crc12_table[256];

void
init_crc12(uint16_t key)
{
	int i, j;

	for (i = 0; i < 256; i++) {
		int reg = i << 4;
		for (j = 0; j < 8; j++) {
			reg <<= 1;
			if (reg & 0x1000)
				reg ^= key;
		}
		crc12_table[i] = reg;
	}
}

uint16_t
crc12(uint16_t crc, void *buf, size_t len)
{
	uint8_t *p = buf;
	int i;

	for (i = 0; i < len; i++) {
		uint8_t top = crc >> 4;
		crc = ((crc << 8) + p[i ^ 3]) ^ crc12_table[top];
	}

	return (crc & 0x0fff);
}

uint32_t *frame;
size_t framelen;

void update_mk4_frame(time_t);
void update_mk5b_frame(time_t);
void (*update_frame)(time_t);

#define MK4_TRACK_FRAME_SIZE	2500
#define MK4_TRACK_FRAME_WORDS	(MK4_TRACK_FRAME_SIZE / sizeof(uint32_t))

uint32_t header[5];
int ntracks;

void
init_mk4_frame(void)
{
	int i;

	framelen = ntracks * MK4_TRACK_FRAME_SIZE;
	frame = malloc(framelen);
	if (frame == NULL)
		fatal("malloc");

	header[2] = 0xffffffff;
	for (i = ntracks * 2; i < ntracks * 3; i++)
		frame[i] = 0xffffffff;

	for (i = ntracks * 5; i < ntracks * MK4_TRACK_FRAME_WORDS; i++)
		frame[i] = 0x11223344;

	init_crc12(0x80f);
	update_frame = update_mk4_frame;
}

void
update_mk4_frame(time_t clock)
{
	uint8_t *frame8 = (uint8_t *)frame;
	uint16_t *frame16 = (uint16_t *)frame;
	uint32_t *frame32 = (uint32_t *)frame;
	uint64_t *frame64 = (uint64_t *)frame;
	struct tm tm;
	int i, j;

	gmtime_r(&clock, &tm);
	tm.tm_yday += 1;

	header[3] = 0;
	header[3] |= (tm.tm_year / 1) % 10 << 28;
	header[3] |= (tm.tm_yday / 100) % 10 << 24;
	header[3] |= (tm.tm_yday / 10) % 10 << 20;
	header[3] |= (tm.tm_yday / 1) % 10 << 16;
	header[3] |= (tm.tm_hour / 10) % 10 << 12;
	header[3] |= (tm.tm_hour / 1) % 10 << 8;
	header[3] |= (tm.tm_min / 10) % 10 << 4;
	header[3] |= (tm.tm_min / 1) % 10 << 0;

	header[4] = 0;
	header[4] |= (tm.tm_sec / 10) % 10 << 28;
	header[4] |= (tm.tm_sec / 1) % 10 << 24;
	header[4] |= crc12(0, &header, sizeof(header));

	switch (ntracks) {
	case 8:
		for (i = 3; i < 5; i++) {
			for (j = 0; j < 32; j++) {
				if (header[i] & (1 << (31 - j)))
					frame8[(i * 32) + j] = ~0;
				else
					frame8[(i * 32) + j] = 0;
			}
		}
		break;
	case 16:
		for (i = 3; i < 5; i++) {
			for (j = 0; j < 32; j++) {
				if (header[i] & (1 << (31 - j)))
					frame16[(i * 32) + j] = ~0;
				else
					frame16[(i * 32) + j] = 0;
			}
		}
		break;
	case 32:
		for (i = 3; i < 5; i++) {
			for (j = 0; j < 32; j++) {
				if (header[i] & (1 << (31 - j)))
					frame32[(i * 32) + j] = ~0;
				else
					frame32[(i * 32) + j] = 0;
			}
		}
		break;
	case 64:
		for (i = 3; i < 5; i++) {
			for (j = 0; j < 32; j++) {
				if (header[i] & (1 << (31 - j)))
					frame64[(i * 32) + j] = ~0;
				else
					frame64[(i * 32) + j] = 0;
			}
		}
		break;
	}
}

#define MK5B_FRAME_WORDS	2500
#define MK5B_FRAME_SIZE		(MK5B_FRAME_WORDS * sizeof(uint32_t))

void
init_mk5b_frame(void)
{
	int i;

	framelen = MK5B_FRAME_SIZE;
	frame = malloc(framelen);
	if (frame == NULL)
		fatal("malloc");

	frame[0] = 0xabaddeed;
	frame[1] = 0x00000000;
	frame[2] = 0x00000000;
	frame[3] = 0x00000000;

	for (i = 4; i < MK5B_FRAME_WORDS; i++)
		frame[i] = 0x11223344;

	update_frame = update_mk5b_frame;
}

void
update_mk5b_frame(time_t clock)
{
	int mjd, sec;
	uint32_t word;

	mjd = 40587 + (clock / 86400);
	sec = clock % 86400;

	word = 0;
	word |= ((sec / 1) % 10) << 0;
	word |= ((sec / 10) % 10) << 4;
	word |= ((sec / 100) % 10) << 8;
	word |= ((sec / 1000) % 10) << 12;
	word |= ((sec / 10000) % 10) << 16;
	word |= ((mjd / 1) % 10) << 20;
	word |= ((mjd / 10) % 10) << 24;
	word |= ((mjd / 100) % 10) << 28;
	frame[2] = word;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d] [-f format] [-p port]\n", __progname);
	exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
	extern char *__progname;
	struct sockaddr_un sun;
	struct sockaddr_in sin;
	struct mk5read_msg msg;
	char *format = NULL;
	char *buf;
	size_t len;
	socklen_t slen;
	ssize_t nbytes;
	int rcvbuf;
	int s, t, fd;
	struct pollfd pfd[1];
	int nfds;
	int ntimeouts = 0;
	time_t clock;
	int ch;

	signal(SIGPIPE, SIG_IGN);

	while ((ch = getopt(argc, argv, "df:p:")) != -1) {
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'f':
			format = strdup(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}

	argc -= optind;
	argv += optind;
	if (argc != 0)
		usage();

	if (format) {
		if (strncmp(format, "mark4", 5) == 0 && format[5] == ':') {
			ntracks = atoi(&format[6]);
			format[5] = 0;
		} else if (strcmp(format, "mark5b") == 0) {
			ntracks = 32;
		} else {
			fprintf(stderr, "%s: invalid format '%s'\n",
			    __progname, format);
			exit(EXIT_FAILURE);
		}
		if (ntracks != 8 && ntracks != 16 &&
		    ntracks != 32 && ntracks != 64) {
			fprintf(stderr, "%s: invalid number of tracks\n",
			    __progname);
			exit(EXIT_FAILURE);
		}
	}

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		fatal("socket");

	rcvbuf = 1024 * 1024;
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) == -1)
		fatal("setsockopt");

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		fatal("bind");

	len = 16384;
	buf = malloc(len);
	if (buf == NULL)
		fatal("malloc");

	t = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (t == -1)
		fatal("socket");

	sun.sun_family = AF_LOCAL;
	strlcpy(sun.sun_path, MK5READ_SOCKET, sizeof(sun.sun_path));
	unlink(MK5READ_SOCKET);
	if (bind(t, (struct sockaddr *)&sun, sizeof(sun)) == -1)
		fatal("bind");

	if (listen(t, 1) == -1)
		fatal("listen");

	for (;;) {
		slen = sizeof(sun);
		fd = accept(t, (struct sockaddr *)&sun, &slen);
		if (fd == -1)
			fatal("accept");

		if (read(fd, &msg, sizeof(msg)) != sizeof(msg))
			fatal("read");

		if (debug)
			printf("%s:%llu\n", msg.vsn,
			    (unsigned long long)msg.off);

		if (format == NULL) {
			format = msg.vsn;
			ntracks = msg.off;
		}

		if (strcmp(format, "mark4") == 0 &&
		    (ntracks == 8 || ntracks == 16 ||
		     ntracks == 32 || ntracks == 64))
			init_mk4_frame();
		else if (strcmp(format, "mark5b") == 0)
			init_mk5b_frame();
		else {
			fprintf(stderr, "invalid format %s:%d\n", format, ntracks);
			close(fd);
			continue;
		}

		pfd[0].fd = s;
		pfd[0].events = POLLIN;

		for (;;) {
			nfds = poll(pfd, 1, 1000);
			if (nfds == 0) {
				if (ntimeouts++ > 1) {
					update_frame(clock);
					nbytes = write(fd, frame, framelen);
					if (nbytes != framelen)
						break;
					printf("fake frame sent for %s", ctime(&clock));
				}
				clock = time(NULL);
				continue;
			}

			nbytes = recv(s, buf, len, 0);
			if (nbytes == -1)
				fatal("read");
			//printf("seqno %lld\n", *(uint64_t *)buf);

			len = nbytes;
			nbytes = write(fd, buf + 8, len - 8);
			if (nbytes != (ssize_t)(len - 8))
				break;

			ntimeouts = 0;
		}

		ntimeouts = 0;
		close(fd);
	}

	if (unlink(MK5READ_SOCKET) == -1)
		fatal("unlink");
	return (EXIT_SUCCESS);
}
