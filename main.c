#include <asm/errno.h>
#include <stdio.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <net/if.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "disconnect.h"
#include "errors.h"

static char error[STR_LEN];

static int nl80211_msg_error_handler(struct sockaddr_nl *nla,
				    struct nlmsgerr *err, void *arg) {
	int *ret = arg;
	*ret = err->error;

	fprintf(stderr, "An error has occurred: %s\n", nl_geterror(*ret));
	return NL_STOP;
}

static int nl80211_msg_finish_handler(struct nl_msg *msg, void *arg) {
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int nl80211_msg_ack_handler(struct nl_msg *msg, void *arg) {
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static int nl80211_process_events(struct nl_msg *msg, void *arg) {
	struct genlmsghdr *gnlh;
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nl_msg_dump(msg, stdout);
	return 0;
}

static int get_wifi_messages(struct nl80211_state *state, char *ifname) {
	int idx;
	int err;

	struct nl_msg *msg;

	idx = if_nametoindex(ifname);

	msg = nlmsg_alloc();
	if (!msg) {
		err = errno;
		strcpy(error, error_messages[ERR_MSG_CREATE]);
		_handle_errors(error, err);
		nlmsg_free(msg);
		return EXIT_FAILURE;
	}

	genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, state->id, 0, NLM_F_DUMP, 0, 0);
	nla_put_u32(msg, NL80211_ATTR_IFINDEX, idx);

	//nl_send_auto(state->socket, msg);

	err = nl_send_auto_complete(state->socket, msg);
	if (err < 0) {
		fprintf(stderr, "nl_send_complete_auto() returned an error: %s",
			nl_geterror(err));
		return -err;
	}

	while (1)
		nl_recvmsgs(state->socket, state->callback);

	nlmsg_free(msg);
	return 0;
}
static int nl80211_init(struct nl80211_state *state) {
	int err;

	state = malloc(sizeof(struct nl80211_state));
	if (!state) {
		err = errno;
		strcpy(error, error_messages[ERR_NL80211]);
        	_handle_errors(error, err);
        	goto cleanup;
	}

	memset(state, 0, sizeof(*state));

	state->socket = nl_socket_alloc();
	if (!state->socket) {
		err = errno;
		strcpy(error, error_messages[ERR_SOCKET]);
        	_handle_errors(error, err);
		goto cleanup;
	}

	if (genl_connect(state->socket)) {
		err = errno;
		strcpy(error, error_messages[ERR_NETLINK]);
        	_handle_errors(error, err);
		goto cleanup;
	}
	nl_socket_set_buffer_size(state->socket, 8192, 8192);

	state->id = genl_ctrl_resolve(state->socket, "nl80211");
	if (!state->id) {
		strcpy(error, error_messages[ERR_RESOLVE]);
		_handle_errors(error, err);
		err = errno;
		goto cleanup;
	}

	state->callback = nl_cb_alloc(NL_CB_DEFAULT);
	if (!state->callback) {
		err = errno;
		strcpy(error, error_messages[ERR_CB_ALLOCATE]);
		_handle_errors(error, err);
		goto cleanup;
	}

	nl_socket_set_cb(state->socket, state->callback);

	nl_cb_set(state->callback, NL_CB_VALID , NL_CB_CUSTOM, nl80211_process_events, NULL);
	nl_cb_set(state->callback, NL_CB_FINISH, NL_CB_CUSTOM, nl80211_msg_finish_handler, NULL);
	nl_cb_err(state->callback, NL_CB_CUSTOM, nl80211_msg_error_handler, &err);
	nl_cb_set(state->callback, NL_CB_ACK, NL_CB_CUSTOM, nl80211_msg_ack_handler, NULL);

	return 0;

	cleanup:
		if (state->socket) {
			nl_close(state->socket);
			nl_socket_free(state->socket);
		}
		free(state);
		return -err;
}

int main(int argc, char **argv) {
	int o;
	int err;

	struct nl80211_state state;

	int frequency = 0;
	char *ifname = NULL;

	while ((o = getopt(argc, argv, "f:i")) != -1) {
		switch (o) {
			case 'f':
				frequency = atoi(optarg);
				break;
			case 'i':
				ifname = optarg;
				break;
			case '?':
				fprintf(stderr, "Unknown option '%c'\n", optopt);
				return 1;
			default:
				fprintf(stdout, "Usage: disconnect -f <frequency>\n");
				return 1;
		}
	}

	err = nl80211_init(&state);

	if (err < 0) {
		fprintf(stdout, "Could not initialize nl80211 state.\n");
		return EXIT_FAILURE;
	}
	while (1) {
		get_wifi_messages(&state, ifname);
		sleep(frequency);
	}

	return 0;
}