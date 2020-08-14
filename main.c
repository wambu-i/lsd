#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>

#include "disconnect.h"
#include "errors.h"

static char error[STR_LEN];

static struct nl80211_state *state = NULL;

static struct nla_policy signal_policy[NL80211_STA_INFO_MAX + 1] = {
	[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
};

static int nl80211_msg_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
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
	struct nlattr *attr[NL80211_ATTR_MAX + 1];
	struct nlattr *info[NL80211_STA_INFO_MAX + 1];
	struct station_info_entry *entry;

	struct station_info_buffer *tmp = arg;

	entry = tmp->buf;

	entry += tmp->len;
	memset(entry, 0, sizeof(*entry));

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	//nl_msg_dump(msg, stdout);

	nla_parse(attr, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (attr[NL80211_ATTR_MAC])
		memcpy(entry->mac, nla_data(attr[NL80211_ATTR_MAC]), 6);

	if (attr[NL80211_ATTR_STA_INFO] &&
	    !nla_parse_nested(info, NL80211_STA_INFO_MAX,
	                      attr[NL80211_ATTR_STA_INFO], signal_policy)) {
		if (info[NL80211_STA_INFO_SIGNAL])
			entry->signal = nla_get_u8(info[NL80211_STA_INFO_SIGNAL]);
		}

	tmp->len++;

	return NL_SKIP;
}

static int nl80211_init() {
	int err;
	int fd;

	if (!state) {
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

		state->family = genl_ctrl_resolve(state->socket, "nl80211");
		if (state->family < 0) {
			err = errno;
			strcpy(error, error_messages[ERR_RESOLVE]);
			_handle_errors(error, err);
			goto cleanup;
		}

		fd = nl_socket_get_fd(state->socket);
		if (fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0) {
			err = -EINVAL;
			goto cleanup;
		}

		if (genl_ctrl_alloc_cache(state->socket, &state->cache)) {
			err = errno;
			strcpy(error, error_messages[ERR_GENL_CACHE]);
			_handle_errors(error, err);
			goto cleanup;
		}

		state->nl80211 = genl_ctrl_search_by_name(state->cache, "nl80211");
		if (!state->nl80211) {
			err = errno;
			strcpy(error, error_messages[ERR_GENL_SEARCH]);
			_handle_errors(error, err);
			goto cleanup;
		}

		state->nlctrl = genl_ctrl_search_by_name(state->cache, "nlctrl");
		if (!state->nlctrl) {
			err = errno;
			strcpy(error, error_messages[ERR_GENL_SEARCH]);
			_handle_errors(error, err);
			goto cleanup;
		}
		nl_socket_add_membership(state->socket, state->family);
	}
	return 0;

	cleanup:
		if (state->socket) {
			nl_close(state->socket);
			nl_socket_free(state->socket);
		}
		free(state);
		return -err;
}


static struct nl80211_msg *nl80211_new(struct genl_family *family, int cmd, int flags) {
	int err;
	static struct nl80211_msg msg;

	struct nl_msg *req = NULL;
	struct nl_cb *callback = NULL;

	req = nlmsg_alloc();
	if (!req) {
		err = errno;
		strcpy(error, error_messages[ERR_MSG_CREATE]);
		_handle_errors(error, err);
		nlmsg_free(req);
		return NULL;
	}

	callback = nl_cb_alloc(NL_CB_DEFAULT);
	if (!callback) {
		err = errno;
		strcpy(error, error_messages[ERR_CB_ALLOCATE]);
		_handle_errors(error, err);
		nlmsg_free(req);
		return NULL;
	}

	genlmsg_put(req, 0, 0, genl_family_get_id(family), 0, flags, cmd, 0);

	msg.msg = req;
	msg.callback = callback;

	return &msg;
}

static struct nl80211_msg *nl80211_msg_setup(const char *ifname,
                                                 int cmd, int flags) {
	struct nl80211_msg *msg;

	int idx = -1;

	if (ifname == NULL) {
		_handle_error("No interface specified");
		return NULL;
	}

	if (nl80211_init() < 0) {
		printf("nl80211 init failed\n");
		return NULL;
	}

	idx = if_nametoindex(ifname);

	if (idx <= 0) {
		printf("Could not resolve interface name\n");
		return NULL;
	}

	msg = nl80211_new(state->nl80211, cmd, flags);
	if (!msg) {
		printf("Could not set up new msg\n");
		return NULL;
	}

	if (idx > 0)
		nla_put_u32(msg->msg, NL80211_ATTR_IFINDEX, idx);

	return msg;
}

static int get_associated_stations(struct station_info_buffer *buffer, char *ifname) {
	int err;

	struct nl80211_msg *msg;

	msg = nl80211_msg_setup(ifname, NL80211_CMD_GET_STATION, NLM_F_DUMP);

	if (!msg) {
		return -1;
	}

	nl_cb_set(msg->callback, NL_CB_VALID, NL_CB_CUSTOM, nl80211_process_events, buffer);

	err = nl_send_auto_complete(state->socket, msg->msg);

	if (err < 0) {
		if (msg->callback)
			nl_cb_put(msg->callback);
		if (msg->msg)
			nlmsg_free(msg->msg);
		return -1;
	}

	err = 1;

	nl_cb_err(msg->callback, NL_CB_CUSTOM, nl80211_msg_error_handler,  &err);
	nl_cb_set(msg->callback, NL_CB_FINISH, NL_CB_CUSTOM, nl80211_msg_finish_handler, &err);
	nl_cb_set(msg->callback, NL_CB_ACK, NL_CB_CUSTOM, nl80211_msg_ack_handler, &err);

	while (err > 0)
		nl_recvmsgs(state->socket, msg->callback);

	return 0;
}

int main(int argc, char **argv) {
	int o;
	int err;
	char tmp[1024];

	struct station_info_buffer buffer;
	struct station_info_entry *entry;

	int devs = 0;
	int loops = 0;
	int frequency = 0;
	int threshold = 0;
	char *ifname = NULL;


	while ((o = getopt(argc, argv, "f:i:t:l:")) != -1) {
		switch (o) {
			case 'f':
				frequency = atoi(optarg);
				break;
			case 'i':
				ifname = optarg;
				break;
			case 't':
				threshold = atoi(optarg);
				break;
			case 'l':
				loops = atoi(optarg);
				break;
			case '?':
				fprintf(stderr, "Unknown option '%c'\n", optopt);
				return 1;
			default:
				fprintf(stdout, "Usage:\n disconnect -i <interface> -f <frequency> -t <threshold>\n");
				return 1;
		}
	}

	buffer.buf = tmp;
	buffer.len = 0;

	//while (1) {

	for (int i = 0; i < loops; i++) {
		err = get_associated_stations(&buffer, ifname);

		if (err) {
			fprintf(stdout, "No client information available\n");
			return -1;
		}
		else if (buffer.len <= 0) {
			fprintf(stdout, "No stations connected\n");
			return -1;
		}

		for (int i = 0; i < buffer.len; i += sizeof(struct station_info_entry)) {
			entry = (struct station_info_entry *) &buffer.buf[i];

			fprintf(stdout, "Client: %02X:%02X:%02X:%02X:%02X:%02X - Signal: ", entry->mac[0], entry->mac[1],
					entry->mac[2], entry->mac[3], entry->mac[4], entry->mac[5]);
			if (!entry->signal)
				fprintf(stdout, "unknown signal\n");
			else
				fprintf(stdout, "%d dBm\n", entry->signal);

			if (abs(entry->signal) < threshold)
				devs++;
		}

		fprintf(stdout, "%d client(s) under threshold signal.\n", devs);
		devs = 0;
		sleep(frequency);
	}

	//
	//sleep(frequency);
	//}

	return 0;
}