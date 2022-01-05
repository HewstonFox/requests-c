#pragma once

#include <ctype.h>
#include <curl/curl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct res_header_s {
    char *name;
    char *value;
} RCHeader;

typedef struct res_headers_s {
    size_t len;
    size_t cap;
    RCHeader **headers;
} RCHeaders;

typedef struct res_data_s {
    void *body;
    RCHeaders *headers;
    long code;
    double elapsed;
    char *url;
    size_t body_cap;
    size_t body_len;
} RCResponse;

void RC_init();

void RC_cleanup();

void RC_response_destroy(RCResponse *resp);

RCHeader *RC_create_header(const char *name, const char *value);

RCHeader *RC_find_header(RCHeaders* h, const char *name);

void RC_destroy_header(RCHeader *h);

RCHeaders *RC_create_headers();

void RC_destroy_headers(RCHeaders *h);

void RC_set_header(RCHeaders *h, const char *name, const char *value);

char *RC_get_header(RCHeaders *h, const char *name);

void RC_remove_header(RCHeaders *h, const char *name);

RCResponse * RC_send_request(const char *method, const char *url, RCHeaders *headers, void *body, size_t body_len);

#define RC_POST(url, headers, body, body_len) RC_send_request("POST", url, headers, body, body_len)
#define RC_GET(url, headers, body, body_len) RC_send_request("GET", url, headers, body, body_len)
#define RC_PUT(url, headers, body, body_len) RC_send_request("PUT", url, headers, body, body_len)
#define RC_DELETE(url, headers, body, body_len) RC_send_request("DELETE", url, headers, body, body_len)
#define RC_HEAD(url, headers, body, body_len) RC_send_request("HEAD", url, headers, body, body_len)
#define RC_OPTIONS(url, headers, body, body_len) RC_send_request("OPTIONS", url, headers, body, body_len)
#define RC_PATCH(url, headers, body, body_len) RC_send_request("PATCH", url, headers, body, body_len)
