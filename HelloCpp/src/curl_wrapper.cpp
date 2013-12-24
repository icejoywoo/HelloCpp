/*
 * curl_wrapper.cpp
 *
 *  Created on: 2013年12月15日
 *      Author: icejoywoo
 */

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include "curl/curl.h"
#include "cJSON.h"

class Curly{
private:
    std::string        mContent;
    std::string        mType;
    std::vector<std::string> mHeaders;
    unsigned int       mHttpStatus;
    CURL*              pCurlHandle;
    static size_t      HttpContent(void* ptr, size_t size,
                        size_t nmemb, void* stream);
    static size_t      HttpHeader(void* ptr, size_t size,
                        size_t nmemb, void* stream);

public:
    Curly(): pCurlHandle(curl_easy_init()) {};  // constructor
    ~Curly(){};
    CURLcode    Fetch (std::string);

    inline std::string   Content()    const { return mContent; }
    inline std::string   Type()       const { return mType; }
    inline unsigned int  HttpStatus() const { return mHttpStatus; }
    inline std::vector<std::string> Headers()    const { return mHeaders; }
};

CURLcode Curly::Fetch(std::string url){

    // clear things ready for our 'fetch'
    mHttpStatus = 0;
    mContent.clear();
    mHeaders.clear();

    // set our callbacks
    curl_easy_setopt(pCurlHandle , CURLOPT_WRITEFUNCTION, HttpContent);
    curl_easy_setopt(pCurlHandle, CURLOPT_HEADERFUNCTION, HttpHeader);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEHEADER, this);

    // set the URL we want
    curl_easy_setopt(pCurlHandle, CURLOPT_URL, url.c_str());

    //  go get 'em, tiger
    CURLcode curlErr = curl_easy_perform(pCurlHandle);
    if (curlErr == CURLE_OK){

        // assuming everything is ok, get the content type and status code
        char* content_type = NULL;
        if ((curl_easy_getinfo(pCurlHandle, CURLINFO_CONTENT_TYPE,
              &content_type)) == CURLE_OK)
            mType = std::string(content_type);

        unsigned int http_code = 0;
        if((curl_easy_getinfo (pCurlHandle, CURLINFO_RESPONSE_CODE,
              &http_code)) == CURLE_OK)
            mHttpStatus = http_code;

    }
    return curlErr;
}

size_t Curly::HttpContent(void* ptr, size_t size,
                            size_t nmemb, void* stream) {

	Curly* handle = (Curly*)stream;
	size_t data_size = size*nmemb;
    if (handle != NULL){
        handle->mContent.append((char *)ptr,data_size);
    }
	return data_size;
}

size_t Curly::HttpHeader(void* ptr, size_t size,
                            size_t nmemb, void* stream) {

    Curly* handle = (Curly*)stream;
	size_t data_size = size*nmemb;
    if (handle != NULL && data_size != 0) {
        std::string header_line((char *)ptr,data_size);
        if (header_line.rfind("\r\n") != 0)
        	handle->mHeaders.push_back(
        			header_line.substr(0, header_line.rfind("\r\n")));
    }
    return data_size;
}

int main (int argc, const char * argv[])
{
    Curly curly;
    if (curly.Fetch("http://api.douban.com/book/subject/1220562?alt=json") == CURLE_OK){

        std::cout << "status: " << curly.HttpStatus() << std::endl;
        std::cout << "type: " << curly.Type() << std::endl;
        std::vector<std::string> headers = curly.Headers();

        for(std::vector<std::string>::iterator it = headers.begin();
                it != headers.end(); it++)
            std::cout << "Header: " << (*it) << std::endl;

        std::cout << "Content:\n" << curly.Content() << std::endl;
    }
    // 解析json
    std::string json = curly.Content();
    cJSON* root = cJSON_Parse(json.c_str());
    cJSON* category = cJSON_GetObjectItem(root,"category");
    cJSON* term = cJSON_GetObjectItem(category, "@term");
    printf("term: %s\n", cJSON_Print(term));
    return 0;
}
