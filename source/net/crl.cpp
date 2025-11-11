#include "crl.h"

size_t writeChunk(void *data, size_t size, size_t nmemb, void *userdata) {
    size_t real_size = size * nmemb;

    Response* response = (Response*) userdata;

    char *ptr = (char*)realloc(response->string, response->size + real_size + 1);
    if (ptr == NULL)
        return -1;
    
    response->string = ptr;
    memcpy(&(response->string[response->size]), data, real_size);
    response->size += real_size;
    response->string[response->size] = 0;
    return real_size;
}

// todo: cleanup these methods
std::string sendHTTPRequest(std::string test_url) {
    CURL *curl;
    CURLcode result;
    curl = curl_easy_init();
    std::string debug;

    if (curl == NULL) {
        err("HTTP request failed.");
        return "";
    }

    Response response;
    response.string = (char*)malloc(1);
    response.size = 0;

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, test_url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeChunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        debug = curl_easy_strerror(result);
        err(debug.c_str());
        return "";
    }
    
    debug = response.string;

    curl_easy_cleanup(curl);

    free(response.string);

    return debug;
}

std::string sendHTTPPost(
  const std::string& url,
  const std::string& body,
  const std::vector<std::string>& extraHeaders
) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    err("curl init failed");
    return "";
  }

  // set headers
  struct curl_slist* headers = nullptr;
  for (const auto& h : extraHeaders) {
    headers = curl_slist_append(headers, h.c_str());
  }

  Response resp;
  resp.string = (char*)malloc(1);
  resp.size = 0;

  char errbuf[CURL_ERROR_SIZE]; errbuf[0] = 0;

  // todo: move tihs into helper
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeChunk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&resp);

  // timeouts
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

  CURLcode rc = curl_easy_perform(curl);

  long httpCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

  std::string out;
  if (rc != CURLE_OK) {
    const char* emsg = errbuf[0] ? errbuf : curl_easy_strerror(rc);
    err(emsg);
  } else {
    out = resp.string ? std::string(resp.string) : std::string();
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  if (resp.string) free(resp.string);

  return out;
}