// SPDX-License-Identifier: GPL-2.0
/*
 * mbox-demo.c — userspace test for cv1800 mailbox demo
 *
 * Sends a test message via debugfs and reads it back.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>

#define SEND_PATH "/sys/kernel/debug/cv1800_mbox_demo/send"
#define RECV_PATH "/sys/kernel/debug/cv1800_mbox_demo/recv"
#define COUNT_PATH "/sys/kernel/debug/cv1800_mbox_demo/count"

int main(void)
{
	const char *test_msg = "CV1800!";
	char buf[16] = {0};
	int fd, ret, n, count_before, count_after;

	fd = open(COUNT_PATH, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "FAIL: cannot open count (%s)\n", strerror(errno));
		fprintf(stderr, "Is cv1800_mbox_demo module loaded?\n");
		return 1;
	}
	if ((n = read(fd, buf, sizeof(buf) - 1)) < 0) { perror("read"); }
	count_before = atoi(buf);
	close(fd);

	fd = open(SEND_PATH, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "FAIL: cannot open send (%s)\n", strerror(errno));
		return 1;
	}
	ret = write(fd, test_msg, strlen(test_msg));
	close(fd);
	if (ret < 0) {
		fprintf(stderr, "FAIL: send error (%s)\n", strerror(errno));
		return 1;
	}

	fd = open(COUNT_PATH, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "FAIL: cannot open count\n");
		return 1;
	}
	if ((n = read(fd, buf, sizeof(buf) - 1)) < 0) { perror("read"); }
	count_after = atoi(buf);
	close(fd);

	if (count_after != count_before + 1) {
		fprintf(stderr, "FAIL: count did not increment (%d -> %d)\n",
			count_before, count_after);
		return 1;
	}

	memset(buf, 0, sizeof(buf));
	fd = open(RECV_PATH, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "FAIL: cannot open recv (%s)\n", strerror(errno));
		return 1;
	}
	if ((n = read(fd, buf, sizeof(buf) - 1)) < 0) { perror("read"); }
	close(fd);

	if (strncmp(buf, test_msg, 7) == 0) {
		printf("PASS: sent='%s' recv='%s' count=%d\n",
		       test_msg, buf, count_after);
		return 0;
	}

	fprintf(stderr, "FAIL: sent='%s' recv='%s'\n", test_msg, buf);
	return 1;
}
