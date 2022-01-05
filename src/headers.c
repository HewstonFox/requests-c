#include "requests-c.h"

int strcmpi(const char *s1, const char *s2) {
    while (*s1 && tolower(*s1) == tolower(*s2)) {
        s1++;
        s2++;
    }
    return tolower(*s1) - tolower(*s2);
}

RCHeader *RC_create_header(const char *name, const char *value) {
    RCHeader *h = (RCHeader *)malloc(sizeof(RCHeader));
    h->name = strdup(name);
    h->value = strdup(value);
    return h;
}

static ssize_t RC_get_header_idx(RCHeaders *h, const char *name) {
    for (size_t i = 0; i < h->len; i++)
        if (!strcmpi(name, h->headers[i]->name))
            return i;
    return -1;
}

RCHeader *RC_find_header(RCHeaders *h, const char *name) {
    ssize_t idx = RC_get_header_idx(h, name);
    if (idx == -1)
        return NULL;
    return h->headers[idx];
}

void RC_destroy_header(RCHeader *h) {
    free(h->name);
    free(h->value);
    free(h);
}

RCHeaders *RC_create_headers() {
    RCHeaders *h = (RCHeaders *)malloc(sizeof(RCHeaders));
    h->len = 0;
    h->cap = 10;
    h->headers = (RCHeader **)calloc(h->cap, sizeof(RCHeader *));
    memset(h->headers, 0, sizeof(RCHeader *) * h->cap);
    return h;
}

void RC_destroy_headers(RCHeaders *h) {
    for (size_t i = 0; i < h->len; i++)
        RC_destroy_header(h->headers[i]);
    free(h->headers);
    free(h);
}

void RC_set_header(RCHeaders *h, const char *name, const char *value) {
    RCHeader *existing_header = RC_find_header(h, name);
    if (existing_header) {
        free(existing_header->value);
        existing_header->value = strdup(value);
        return;
    }
    h->headers[h->len++] = RC_create_header(name, value);
    if (h->len == h->cap) {
        h->cap *= 2;
        RCHeader **new_headers = (RCHeader **)calloc(h->cap, sizeof(RCHeader *));
        memset(h->headers, 0, sizeof(RCHeader *) * h->cap);
        memcpy(new_headers, h->headers, h->len);
        free(h->headers);
        h->headers = new_headers;
    }
}

char *RC_get_header(RCHeaders *h, const char *name) {
    RCHeader *header = RC_find_header(h, name);
    if (header)
        return header->value;
    return NULL;
}

void RC_remove_header(RCHeaders *h, const char *name) {
    ssize_t idx = RC_get_header_idx(h, name);
    if (idx == -1)
        return;
    RC_destroy_header(h->headers[idx]);
    for (size_t i = idx; i < h->len - 1; i++)
        h->headers[i] = h->headers[i + 1];
    h->len--;
}
