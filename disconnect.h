
#ifndef DISCONNECT_H
#define DISCONNECT_H

#include <json-c/json_object.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#define ETH_ALEN 6

#ifndef NETLINK_EXT_ACK
#define NETLINK_EXT_ACK 11
#endif

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

struct nl80211_state {
	int id;
	struct nl_cb *callback;
	struct nl_sock *socket;
};

#endif
