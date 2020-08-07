#include <asm/errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "disconnect.h"
#include "errors.h"

static char error[STR_LEN];

static int nl80211_init() {
	struct nl80211_state *state;

	state = malloc(sizeof(struct nl80211_state));
	if (!state) {
		err = errno;
		strcpy(error, error_messages[ERR_NL80211]);
        	_handle_errors(error, err);
        	goto cleanup;
	}

	memset(state, 0, sizeof(*state));

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		err = errno;
		strcpy(error, error_messages[ERR_SOCKET]);
        	_handle_errors(error, err);
		goto cleanup;
	}

	if (genl_connect(state->nl_sock)) {
		err = errno
		strcpy(error, error_messages[ERR_NETLINK]);
        	_handle_errors(error, err);
		goto cleanup;
	}

	fd = nl_socket_get_fd(nls->nl_sock);
	if (fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0) {
	err = -EINVAL;
	goto err;
	}

	return 0;

	cleanup:
		nl80211_close();
		return -err;
}

int main(int argc, char **argv) {
	int o;
	int err;
	int frequency = 0;

	while ((o = getopt(argc, argv, "f")) != -1) {
		switch (o) {
			case 'f':
				frequency = atoi(optarg);
				break;
			case '?':
				fprintf(stderr, "Unknown option '%c'\n", optopt);
				return 1;
			default:
				fprintf(stdout, "Usage: disconnect -f <frequency>\n");
				return 1;
		}
	}

	printf("%d\n", frequency);
	return 0;
}