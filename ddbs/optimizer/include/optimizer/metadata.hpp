#pragma once

#include <bits/stdc++.h>
#include <curl/curl.h>
#include <json/json.h>
#include <stdlib.h>
#include "utils.h"

using namespace std;
using namespace Json;

const char *base64_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//三个字节为一组，编码成四个字节
string EncodeBase64(const string inputStr);
string DecodeBase64(const string inputStr);
vector<string> string2list(string inputStr);
string string2json(string key, string value);
string json2string(string inputStr);
size_t write_data(void *buffer, size_t size, size_t nmemb, void *stream);
string etcd_opt(string data, string op);
void putTables();
vector<metadataTable> getTables();
void etcd_set(string key, string value);
string etcd_get(string key);
