#pragma once
#include <stdio.h>
#include <string.h>
#include <vector>

#include <curl/curl.h>
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

enum KEYSPACEPOLICIES {
	NODEFAULT = 0,
	CREATEDEFAULT = 1
};

struct DBKeySpace : std::vector<std::string> {};

struct DBTestStruct {
	std::string name;
	std::string email;
};

class DBConnector {

	struct Memory {
		char* response;
		size_t size;
	};
public:

	DBConnector(const char* serverIP, DBKeySpace keySpace);
	~DBConnector() = default;

	template<typename T>
	void addToData(const char* key, T value) {
		data[key] = value;
	}

	void sendData(const char* subdirectory, KEYSPACEPOLICIES keySpacePolicy = NODEFAULT);

	bool getData(const char* subdirectory, json& output);

	void clearData();
private:
	const char* ipAddress;
	static size_t dataCallback(char* data, size_t size, size_t nmemb, void* clientp);

	json data;
	
	DBKeySpace keySpace;
};