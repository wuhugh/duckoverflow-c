#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <zlib.h>
#include "cJSON.h"
#include "duckutils.h"

/* Test Q ID: 3024197 */

/*  
This struct holds the data received from the server since there
is a chance we do not receive the entire response we want in one go 
*/
typedef struct DataStruct {
    size_t size;
    char* data;
} DataStruct;

/*
Params:
    contents: the data we are receving
    size: size of one data item
    num: number of data items
    userdata: points to the struct we declared to hold the response

This function takes the payload from the server and appends it to our struct
*/
static size_t write_response(void *contents, size_t size, size_t num, void *userdata){
    /* The actual payload size */
    size_t real_size = size * num;
    /* Point to the struct that holds the response */
    DataStruct *data_block = (DataStruct *)userdata;

    /* Make room for incoming data */
    /* The 1 is added at the end because the response we receive is not zero terminated */
    /* i.e we must add the zero ourself */
    data_block->data = realloc(data_block->data, data_block->size + real_size + 1);
    if(data_block->data == NULL){
        /* Not enough memory */
        printf("Not enough memory to write_response\n");
        return 0;
    }

    /* Copy the contents of the payload to our struct */
    memcpy(&(data_block->data[data_block->size]), contents, real_size);
    /* Update the size of our struct */
    data_block->size += real_size;
    /* Zero terminate the data */
    data_block->data[data_block->size] = 0;

    /* Curl requires that we return real_size otherwise an error will be set */
    return real_size;
} 

/* Initializes curl's options to make requests to the API */
static void init_opt(CURL *curl, char *api_url, DataStruct *response_from_server){
    /* Set the URL for curl to make requests to */
    curl_easy_setopt(curl, CURLOPT_URL, api_url);

    /* Recommended by Stackexchange */
    /* The responses are all compressed so we must decompress them */
    /* God bless curl */
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

    /* Point the the place in memory where curl will write the response to */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_from_server);

    /* Pass curl a function to properly write the response */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);

    return;
}

int main(int argc, char* argv[]){
    /* Initialize curl once at the beginning */
    curl_global_init(CURL_GLOBAL_ALL);
    
    /* Create a handle to use */
    CURL *curl = curl_easy_init();
    CURLcode res;
    int i;

    if(curl == NULL){
        printf("curl_easy_init error\n");
        return -1;
    }

    char *api_url = build_request("what does int argc mean"); 

    /* Create a structure to hold the response we get from the call */
    DataStruct response_from_server;
    response_from_server.data = malloc(1);
    response_from_server.size = 0;

    /* Set the CURL OPTIONS */
    init_opt(curl, api_url, &response_from_server);
        

    /* Execute the response */
    res = curl_easy_perform(curl); 
    if(res != CURLE_OK){
        printf("Failed to call curl_easy_perform: %s\n", curl_easy_strerror(res));
    }
    
    cJSON *root = cJSON_Parse(response_from_server.data);
    cJSON *items = cJSON_GetObjectItem(root, "items");
    cJSON *title = NULL;

    for(i = 0; i < cJSON_GetArraySize(items); ++i){
        cJSON *subitem = cJSON_GetArrayItem(items, i);
        title = cJSON_GetObjectItem(subitem, "title");
        char *rendered = cJSON_Print(title);
        printf("Title: %s\n", rendered);
    }
    
    
    /* Cleanup at the end always */
    free(api_url);
    free(response_from_server.data);
    cJSON_Delete(root);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}

