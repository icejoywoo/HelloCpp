/*
 * evhttp_server.cpp
 *
 *  Created on: 2013年12月25日
 *      Author: icejoywoo
 */

#include <cstring>

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <event.h>
#include <evhttp.h>
#include <event2/thread.h>

void generic_request_handler(struct evhttp_request *req, void *arg) {
	struct evbuffer *returnbuffer = evbuffer_new();
	char *decode_uri = strdup((char*) evhttp_request_uri(req));
	struct evkeyvalq http_query;
	evhttp_parse_query(decode_uri, &http_query);
	printf("%s\n", decode_uri);
	free(decode_uri);
	const char *param_a = evhttp_find_header(&http_query,"a");
	printf("a=%s\n", param_a);
	evbuffer_add_printf(returnbuffer, "Thanks for the request!");
	evhttp_send_reply(req, HTTP_OK, "OK", returnbuffer);
	evhttp_clear_headers(&http_query);
	evbuffer_free(returnbuffer);
	return;
}

void hello_request_handler(struct evhttp_request *req, void *arg) {
	struct evbuffer *returnbuffer = evbuffer_new();

	evbuffer_add_printf(returnbuffer, "Thanks for the request!");
	evhttp_send_reply(req, HTTP_OK, "OK", returnbuffer);
	evbuffer_free(returnbuffer);
	return;
}

int main(int argc, char **argv) {
	unsigned short http_port = 8081;
	const char *http_addr = "0.0.0.0";
	struct evhttp *http_server = NULL;

	event_init();
	evthread_use_pthreads();
	http_server = evhttp_start(http_addr, http_port);
	evhttp_set_cb(http_server, "/hello", hello_request_handler, NULL);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);

	fprintf(stderr, "Server started on port %d\n", http_port);
	event_dispatch();
	evhttp_free(http_server);
	return (0);
}

