#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "libwebsockets.h"

#define TESTER INSTALL_DATADIR
#define MAX_MESSAGE_QUEUE 32

#pragma region Prototypes

static int callback_http(
	struct libwebsocket_context *context,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user,
	void *in,
	size_t len
);

static int callback_lws_mirror(
	struct libwebsocket_context *context,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user,
	void *in,
	size_t len
);

const char *get_mimetype(const char *file);

static void dump_handshake_info(struct libwebsocket *wsi);

#pragma endregion

#pragma region Structures

struct per_session_data_http {
	int fd;
};

struct per_session_data__lws_mirror {
	struct libwebsocket *wsi;
	int ringbuffer_tail;
};

struct a_message {
    void *payload;
    size_t len;
};

static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */
	{
		"http-only",							// name
		callback_http,							// callback
		sizeof(struct per_session_data_http),	// per_session_data_size
		0										// max frame size / rx buffer
	},
	{
		"lws-mirror-protocol",
		callback_lws_mirror,
		sizeof(struct per_session_data__lws_mirror),
		128
	},
	{
		NULL, NULL, 0   /* End of list */
	}
};
#pragma endregion protocols, per_session_data_size, etc.