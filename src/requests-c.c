#include "requests-c.h"

void RC_init() { curl_global_init(CURL_GLOBAL_DEFAULT); }

void RC_cleanup() { curl_global_cleanup(); }

static void increase_body(RCResponse *ctx) {
    ctx->body_cap *= 2;
    unsigned char *tmp = malloc(ctx->body_cap);
    memset(tmp, 0, ctx->body_cap);
    memcpy(tmp, ctx->body, ctx->body_len);
    free(ctx->body);
    ctx->body = tmp;
}

static RCResponse *init_ctx() {
    RCResponse *resp = malloc(sizeof(RCResponse));
    resp->headers = RC_create_headers();
    resp->body_cap = 100;
    resp->body_len = 0;
    resp->body = malloc(resp->body_cap);
    memset(resp->body, 0, resp->body_cap);
    return resp;
}

static size_t res_write(void *ptr, size_t size, size_t nmemb, RCResponse *resp) {
    size_t readED = size * nmemb;
    size_t new_len = readED + resp->body_len;
    while (resp->body_cap <= new_len)
        increase_body(resp);
    memcpy((unsigned char *)resp->body + resp->body_len, ptr, readED);
    resp->body_len += readED;
    return readED;
}

size_t res_header_write(void *ptr, size_t size, size_t nmemb, RCResponse *resp) {
    size_t readED = size * nmemb;
    char *buff = malloc(readED + 1);
    memset(buff, 0, readED + 1);
    memcpy(buff, ptr, readED);

    char *header_name_end = strchr(buff, ':');
    if (!header_name_end) {
        free(buff);
        return readED;
    }

    size_t name_len = header_name_end - buff;
    char *name_buff = malloc(name_len + 1);
    memset(name_buff, 0, name_len + 1);
    memcpy(name_buff, buff, name_len);

    size_t value_len = (strchr(buff, '\r') - buff) - name_len - 2;
    char *value_buff = malloc(value_len + 1);
    memset(value_buff, 0, value_len + 1);
    memcpy(value_buff, buff + name_len + 2, value_len);

    RC_set_header(resp->headers, name_buff, value_buff);

    free(buff);
    free(name_buff);
    free(value_buff);
    return readED;
}

void RC_response_destroy(RCResponse *resp) {
    free(resp->body);
    RC_destroy_headers(resp->headers);
    free(resp);
}

RCResponse *RC_send_request(const char *method, const char *url, RCHeaders *headers, void *body, size_t body_len) {
    CURL *curl = curl_easy_init();
    if (!curl)
        return NULL;

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

    struct curl_slist *req_headers = NULL;
    if (headers) {
        for (size_t i = 0; i < headers->len; i++) {
            RCHeader *h = headers->headers[i];
            char *buff = malloc(strlen(h->name) + strlen(h->value) + 3);
            sprintf(buff, "%s: %s", h->name, h->value);
            req_headers = curl_slist_append(req_headers, buff);
            free(buff);
        }
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req_headers);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

    if (body && body_len) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_len);
    }

    RCResponse *data = init_ctx();

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &res_header_write);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &res_write);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

    curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &data->code);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &data->elapsed);
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &data->url);

    curl_slist_free_all(req_headers);
    curl_easy_cleanup(curl);

    return data;
}
