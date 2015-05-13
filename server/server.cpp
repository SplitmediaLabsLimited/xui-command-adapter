#include "server.h"

#pragma comment(lib, "websockets.lib")

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;
static int close_testing;

#pragma region FreakinProceduralFunctions

static int callback_http(
	struct libwebsocket_context *context,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user,
	void *in,
	size_t len
)
{
    char pathBuf[128];  // Path buffer
    char *other_headers;
    const char *mimetype;
    int n;

	memset(pathBuf, 0, sizeof(pathBuf));

    switch(reason) {
        case LWS_CALLBACK_HTTP:
			dump_handshake_info(wsi);

            GetModuleFileNameA( NULL, pathBuf, sizeof(pathBuf) );

            // Remove the exe from tBuff
            for (int i = sizeof(pathBuf); i > 0; i--)
            {
                if (pathBuf[i] == (char)92)
                {
                    memset(&pathBuf[i], 0, sizeof(pathBuf - i));
                    pathBuf[i] = '\0';
                    break;
                }
            }

			// Handle, uhmmm... stuff with routes... I need this to serve
			// the damn .js files :|
			if (strcmp((const char*)in, "/")) {
				if (*((const char *)in) != '/')
				{
					strcat(pathBuf, "/");
				}
				strcat(pathBuf, "/html");
				strncat(pathBuf, (const char*)in, sizeof(pathBuf));

				// Effin convert the dang forward slash to back slashes
				// because windows is a special child
				for (int i = sizeof(pathBuf); i > 0; i--)
				{
					if (pathBuf[i] == (char)47)
					{
						pathBuf[i] = (char)92;
					}
				}
			}
			else
			{
				strcat(pathBuf, "\\html\\index.html");
			}

            mimetype = get_mimetype(pathBuf);
            pathBuf[sizeof(pathBuf) - 1] = '\0';

            n = 0;
            other_headers = NULL;
    
			try {
				n = libwebsockets_serve_http_file(context, wsi, pathBuf, mimetype, other_headers, n);

				if (n < 0 || ((n > 0) && lws_http_transaction_completed(wsi)))
					return -1; /* error or can't reuse connection: close the socket */
			} catch(int e) {
				// Do nothing for now
			}

            break;

        case LWS_CALLBACK_HTTP_BODY:
            strncpy(pathBuf, (const char*)in, 20);
            pathBuf[20] = '\0';
            if (len < 20)
                pathBuf[len] = '\0';

            lwsl_notice("LWS_CALLBACK_HTTP_BODY: %s... len %d\n",
                    (const char*)pathBuf, (int)len);

            break;

        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            lwsl_notice("LWS_CALLBACK_HTTP_BODY_COMPLETION\n");
            /* the whole of the sent body arrived, close or reuse the connection */
            libwebsockets_return_http_status(context, wsi,
                            HTTP_STATUS_OK, NULL);
    }

    return 0;
}

static int callback_lws_mirror(
	struct libwebsocket_context *context,
	struct libwebsocket *wsi,
	enum libwebsocket_callback_reasons reason,
	void *user,
	void *in,
	size_t len
)
{
	int n;
	struct per_session_data__lws_mirror *pss = (struct per_session_data__lws_mirror *)user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_info("callback_lws_mirror: LWS_CALLBACK_ESTABLISHED\n");
		pss->ringbuffer_tail = ringbuffer_head;
		pss->wsi = wsi;
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		lwsl_notice("mirror protocol cleaning up\n");
		for (n = 0; n < sizeof ringbuffer / sizeof ringbuffer[0]; n++)
			if (ringbuffer[n].payload)
				free(ringbuffer[n].payload);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (close_testing)
			break;
		while (pss->ringbuffer_tail != ringbuffer_head) {

			n = libwebsocket_write(wsi, (unsigned char *)
				   ringbuffer[pss->ringbuffer_tail].payload +
				   LWS_SEND_BUFFER_PRE_PADDING,
				   ringbuffer[pss->ringbuffer_tail].len,
								LWS_WRITE_TEXT);
			if (n < 0) {
				lwsl_err("ERROR %d writing to mirror socket\n", n);
				return -1;
			}
			if (n < ringbuffer[pss->ringbuffer_tail].len)
				lwsl_err("mirror partial write %d vs %d\n",
				       n, ringbuffer[pss->ringbuffer_tail].len);

			if (pss->ringbuffer_tail == (MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((ringbuffer_head - pss->ringbuffer_tail) &
				  (MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 15))
				libwebsocket_rx_flow_allow_all_protocol(
					       libwebsockets_get_protocol(wsi));

			// lwsl_debug("tx fifo %d\n", (ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));

			if (lws_partial_buffered(wsi) || lws_send_pipe_choked(wsi)) {
				libwebsocket_callback_on_writable(context, wsi);
				break;
			}
			/*
			 * for tests with chrome on same machine as client and
			 * server, this is needed to stop chrome choking
			 */
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1);
#endif
		}
		break;

	case LWS_CALLBACK_RECEIVE:

		if (((ringbuffer_head - pss->ringbuffer_tail) &
				  (MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 1)) {
			lwsl_err("dropping!\n");
			goto choke;
		}

		if (ringbuffer[ringbuffer_head].payload)
			free(ringbuffer[ringbuffer_head].payload);

		ringbuffer[ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
						  LWS_SEND_BUFFER_POST_PADDING);
		ringbuffer[ringbuffer_head].len = len;
		memcpy((char *)ringbuffer[ringbuffer_head].payload +
					  LWS_SEND_BUFFER_PRE_PADDING, in, len);
		if (ringbuffer_head == (MAX_MESSAGE_QUEUE - 1))
			ringbuffer_head = 0;
		else
			ringbuffer_head++;

		if (((ringbuffer_head - pss->ringbuffer_tail) &
				  (MAX_MESSAGE_QUEUE - 1)) != (MAX_MESSAGE_QUEUE - 2))
			goto done;

choke:
		lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
		libwebsocket_rx_flow_control(wsi, 0);

//		lwsl_debug("rx fifo %d\n", (ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));
done:
		libwebsocket_callback_on_writable_all_protocol(
					       libwebsockets_get_protocol(wsi));
		break;

	/*
	 * this just demonstrates how to use the protocol filter. If you won't
	 * study and reject connections based on header content, you don't need
	 * to handle this callback
	 */

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		dump_handshake_info(wsi);
		/* you could return non-zero here and kill the connection */
		break;

	default:
		break;
	}

	return 0;
}

const char *get_mimetype(const char *file)
{
    int n = strlen(file);

    if (n < 5)
        return NULL;

    if (!strcmp(&file[n - 4], ".ico"))
        return "image/x-icon";

    if (!strcmp(&file[n - 4], ".png"))
        return "image/png";

    if (!strcmp(&file[n - 5], ".html"))
        return "text/html";

    if (!strcmp(&file[n - 3], ".js"))
        return "text/javascript";

	if (!strcmp(&file[n - 4], ".css"))
		return "text/stylesheet";

    return NULL;
}

static void dump_handshake_info(struct libwebsocket *wsi)
{
    int n = 0;
    char buf[256];
    const unsigned char *c;

    do {
        c = lws_token_to_string((lws_token_indexes) n);
        if (!c) {
            n++;
            continue;
        }

        if (!lws_hdr_total_length(wsi, (lws_token_indexes) n)) {
            n++;
            continue;
        }

        lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes) n);

        fprintf(stderr, "    %s = %s\n", (char *)c, buf);
        n++;
    } while (c);
}
#pragma endregion

// server url will be http://localhost:7877 because.. reasons
int main(void) {
	struct libwebsocket_context *context;
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);
	
	info.port = 7877;
	info.gid = -1;
	info.uid = -1;
	info.protocols = protocols;

	// create libwebsocket context representing this server
	context = libwebsocket_create_context(&info);

	if (context == NULL) {
		fprintf(stderr, "libwebsocket init failed\n");
		return -1;
	}

	printf("starting server...\n");

	// infinite loop, to end this server send SIGTERM. (CTRL+C)
	while (1) {
		libwebsocket_service(context, 50);
		// libwebsocket_service will process all waiting events with their
		// callback functions and then wait 50 ms.
		// (this is a single threaded webserver and this will keep our server
		// from generating load while there are not requests to process)
	}

	libwebsocket_context_destroy(context);

	return 0;
}