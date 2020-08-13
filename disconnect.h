
#ifndef DISCONNECT_H
#define DISCONNECT_H

#include <stdint.h>

#define MAC_LEN 6

struct nl80211_state {
	int family;
	struct nl_sock *socket;
	struct nl_cache *cache;
	struct genl_family *nl80211;
	struct genl_family *nlctrl;
};

struct station_info_entry {
	uint8_t	mac[MAC_LEN];
	int8_t signal;
};

struct nl80211_msg {
	struct nl_msg *msg;
	struct nl_cb *callback;
};

struct station_info_buffer {
	void *buf;
	int len;
};

#endif
