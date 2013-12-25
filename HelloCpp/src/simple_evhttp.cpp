/*
 * simple_evhttp.cpp
 *
 *  Created on: 2013年12月26日
 *      Author: icejoywoo
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <time.h>
#include <string.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>

void now_handler(struct evhttp_request *req, void *arg) {
	struct evbuffer *buf;
	buf = evbuffer_new();

	if (buf == NULL)
		err(1, "failed to creat response buffer");

	char *decode_uri = strdup((char*) evhttp_request_uri(req));
	struct evkeyvalq http_query;
	evhttp_parse_query(decode_uri, &http_query);
	printf("%s\n\n", decode_uri);
	free(decode_uri);

	const char *http_param = evhttp_find_header(&http_query, "param");
	const char *http_opt = evhttp_find_header(&http_query, "opt");
	const char *http_charset = evhttp_find_header(&http_query, "charset");
	const char *http_data = evhttp_find_header(&http_query, "data");

	printf("%s\n\n", http_param);
	if (http_charset != NULL) {
		char *content_type = (char *) malloc(64);
		memset(content_type, '\0', 64);
		sprintf(content_type, "text/plain; charset=%s", http_charset);
		evhttp_add_header(req->output_headers, "Content-Type", content_type);
		free(content_type);
	} else {
		evhttp_add_header(req->output_headers, "Content-Type", "text/plain");
	}
	evhttp_add_header(req->output_headers, "Keep-Alive", "120");
	if (strcmp(http_opt, "put") == 0) {
		int buffer_data_len;
		buffer_data_len = EVBUFFER_LENGTH(req->input_buffer);
		printf("%d\n\n", buffer_data_len);
		if (buffer_data_len > 0) {
			char *buffer_data = (char *) malloc(buffer_data_len + 1);
			memset(buffer_data, '\0', buffer_data_len + 1);
			memcpy(buffer_data, EVBUFFER_DATA(req->input_buffer),
			        buffer_data_len);
			printf("%s\n\n", buffer_data);
			evhttp_add_header(req->output_headers, "Content", "dfdfdfdfd");
			evbuffer_add_printf(buf, "%s", "THIS IS OK");
			free(buffer_data);
		} else {
			evbuffer_add_printf(buf, "%s", "END");
		}
	}
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evhttp_clear_headers(&http_query);
	evbuffer_free(buf);

}
int main(int argc, char **argv) {
	struct evhttp *httpd;

	event_init();
	httpd = evhttp_start("0.0.0.0", 1218);
	if (httpd == NULL) {
		fprintf(stderr, "Start server error:%m\n");
		exit(1);
	}

	evhttp_set_gencb(httpd, now_handler, NULL);

	event_dispatch();

	evhttp_free(httpd);

	return 0;

}

