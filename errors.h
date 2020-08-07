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
	ERR_NL80211
	ERR_NETLINK,
	ERR_SOCKET,
	ERR_MSG_CREATE,
	ERR_MSG_APPEND,
	ERR_MSG_COMPLETE,
	ERR_MSQ_CREATE,
	ERR_MSQ_SET,
	ERR_MSQ_CONNECT,
	ERR_MSQ_LOOP,
	ERR_MSQ_PUB
} err;

static char *error_messages[] =  {
	"Could not allocate nl80211 state struct: ",
	"Could not connect to Netlink: ",
	"Could not allocate Netlink socket: "
	"Could not allocate netlink message: ",
	"Could not append netlink message: ",
	"Could not autocomplete netlink message: ",
	"Could not create new mosquitto client instance: ",
	"Could not set maximum inflight messages option: ",
	"Could not connect to MQTT broker: ",
	"Could not start mosquitto loop: "
};

#endif