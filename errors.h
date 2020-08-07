#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <string.h>

#define STR_LEN 150
#define _handle_error(msg) \
    fprintf(stderr, "%s\n", msg)
#define _handle_errors(msg, err) \
    fprintf(stderr, "%s: %s\n", msg, strerror(err))

#define EPARAMETER "Please provide correct parameter"

typedef enum errors {
	ERR_NL80211,
	ERR_NETLINK,
	ERR_SOCKET,
	ERR_RESOLVE,
	ERR_MSG_CREATE,
	ERR_MSG_APPEND,
	ERR_MSG_COMPLETE,
	ERR_CB_ALLOCATE
} err;

static char *error_messages[] =  {
	"Could not allocate nl80211 state struct: ",
	"Could not connect to Netlink: ",
	"Could not allocate Netlink socket: ",
	"nl80211 interface not found: ",
	"Could not allocate netlink message: ",
	"Could not append netlink message: ",
	"Could not autocomplete netlink message: ",
	"Could not create Netlink callback: ",
};

#endif