#include "db.h"
#include <iostream>
#include <chrono>

DBConnector::DBConnector(const char* serverIP, DBKeySpace keySpace) : ipAddress{ serverIP }, keySpace{ keySpace }
{}

/// <summary>
/// POST data to the server
/// DBConnector verifies whether the full keySpace has been filled out before sending data
/// </summary>
/// <param name="subdirectory"></param>
void DBConnector::sendData(const char* subAddress, KEYSPACEPOLICIES keySpacePolicy) {
    //keyspacepolicy is NOT 0
    if (keySpacePolicy) {
        //Create default values
        for (const std::string& key : keySpace)
        {
            //No key found
            if (data.find(key) == data.end()) {
                data[key] = "";
            }
        }
    }
    CURL* curl;
    CURLcode res;
    std::string s = data.dump();
    const char* postthis = s.c_str();
    curl = curl_easy_init();
    std::string fullAddress = ipAddress + std::string(subAddress);
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fullAddress.c_str());
        //curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/user");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

        /* if we do not provide POSTFIELDSIZE, libcurl calls strlen() by itself */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

#ifdef TEST_DBCONNECTOR
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        struct curl_slist* hs = NULL;
        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);

        /* Perform the request, res gets the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

bool DBConnector::getData(const char* subdirectory, json& output)
{
    Memory chunk = {0, 0};
    CURL* curl;
    CURLcode res;

    std::string fullAddress = ipAddress + std::string(subdirectory);
    curl = curl_easy_init();
    //std::string output = "N/A";
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fullAddress.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1l);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        auto start = std::chrono::high_resolution_clock::now();
        //curl_easy_perform seems to be a blocking function meaning after this function call, the chunk will contain received data
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Time taken to receive file: " << elapsed.count() << " seconds" << std::endl;
        //std::cout << chunk.response;
        if (chunk.size > 0) {
            output = json::parse(std::string(chunk.response));
            try {
                auto& n = output[0]["Network"];
            }
            catch (std::exception e) {
                std::cout << output["message"] << std::endl;
                if (output["message"] == "No Genome To Evaluate") return false;
                else {
                    std::cout << "Unexpected Error Retrieving Genome from Server: " << e.what() << "\n";
                }
            }
            free(chunk.response);
            
        }
        return true;
    }
    return false;
}

void DBConnector::clearData()
{
    data = {};
}

size_t DBConnector::dataCallback(char* data, size_t size, size_t nmemb, void* clientp)
{
    size_t realsize = size * nmemb;
    Memory* mem = (Memory*)clientp;

    void* ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr)
        return 0;  /* out of memory! */

    mem->response = (char*)ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

#ifdef TEST_DBCONNECTOR
int main(void)
{
    DBKeySpace dbkeys{};
    dbkeys.push_back("name");
    dbkeys.push_back("email");

    DBConnector db {"http://127.0.0.1:5000", dbkeys};

    db.addToData<float>("name", 3.12225f);
    db.sendData("/user", CREATEDEFAULT);

    json r{};
    db.getData("/users", r);
    std::cout << r.dump(4) << "\n";
    db.getData("/user_max", r);
    std::cout << r.dump(1) << "\n";

    //std::cout << db.receivedData.dump(4) << std::endl;;
    return 0;
}
#endif