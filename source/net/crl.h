#ifndef CRL_H
#define CRL_H

#include <curl/curl.h>
#include <malloc.h>
#include <vector>
#include <string>
#include "../utils/common.h"

typedef struct {
    char *string;
    size_t size;
} Response;


size_t writeChunk(void *data, size_t size, size_t nmemb, void *userdata);
  std::string sendHTTPRequest(std::string test_url);
  std::string sendHTTPPost(
  const std::string& url,
  const std::string& body,
  const std::vector<std::string>& extraHeaders
);

#endif
