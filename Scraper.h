#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <gumbo.h>
#include <string_view>
#include <algorithm>

using namespace std;

bool iequals(std::string_view a, std::string_view b);
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp);
int DownloadHTML(const string& url, string* html);
void find_buttons_and_images(const GumboNode* node);
void find_all_links(const GumboNode* node, vector<string>& links);
