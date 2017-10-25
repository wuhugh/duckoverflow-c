#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <zlib.h>
#include "cJSON.h"
#include "duckutils.h"

/** Limit the number of questions and answers */
#define RESPONSE_LIMIT 5

/**  
 * This struct holds the data curl writes from the server 
 * The entire response we want may not be written in one go
 * so this struct is continually appended to 
*/
typedef struct DataStruct {
    size_t size; /** The size written when the write_response is called */
    char* data; /** The actual response from the server */
} DataStruct;

/**
 * @brief A callback function used by curl when there is data to be written
 * 
 * Because the entire response from the server may not be written at once
 * this callback is called by curl every time there is data to be written
 * @see https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 * 
 * @param void *contents: the data we are receving
 * @param size_t size: size of one data item
 * @param size_t num: number of data items
 * @param void *userdata: points to the struct we declared to hold the response
 *
 * @return Returns the number of bytes written
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

/**
 * @brief Initializes curl's options to make the proper requests
 * 
 * @param CURL *curl: a curl struct made after using one of it's init functions
 * @param DataStruct *response_from_server: a DataStruct that 
 * can hold the data from the server
 * @see struct DataStruct
 * 
 * @return Does not return any value
*/ 
static void init_opt(CURL *curl, DataStruct *response_from_server){
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
    draw_duck();
    /* Initialize curl once at the beginning */
    curl_global_init(CURL_GLOBAL_ALL);
    
    /* Create a handle to use */
    CURL *curl = curl_easy_init();
    CURLcode res;

    if(curl == NULL){
        fprintf(stderr, "curl_easy_init error\n");
        return -1;
    }

    /* Create a structure to hold the response we get from the call */
    DataStruct response_from_server;
    response_from_server.data = malloc(1);
    response_from_server.size = 0;

    /* Set the CURL OPTIONS */
    init_opt(curl, &response_from_server);

    /* Get the query */
    char *query = NULL;
    if(argc < 2){
        // show_menu();
        query = get_input();
    }
    else {
        query = argv[1];
    }

    /* Construct the search query to send */
    char *api_url = build_search_request(query); 
    free(query);

    /* Set the URL for curl to make requests to */
    curl_easy_setopt(curl, CURLOPT_URL, api_url);

    /* Execute the response */
    res = curl_easy_perform(curl); 
    if(res != CURLE_OK){
        fprintf(stderr, "Failed to call curl_easy_perform: %s\n", curl_easy_strerror(res));
    }

    /* Get ready to parse the JSON response */
    cJSON *root = cJSON_Parse(response_from_server.data); /* Contains the entire JSON body */
    cJSON *results = cJSON_GetObjectItem(root, "items"); /* This is an array of each item */
    
    /* Limit to the number of responses i.e. this is min(GetArraySize, RESPONSE_LIMIT)*/
    int num_question_responses = cJSON_GetArraySize(results) < RESPONSE_LIMIT ? cJSON_GetArraySize(results) : RESPONSE_LIMIT;
    
    /* Populate a list of question titles and their text body */
    char *list_of_question_titles[num_question_responses]; 
    parse_cjson_into_list(results, "title", (void *)list_of_question_titles ,num_question_responses);
    char *list_of_question_bodies[num_question_responses];
    parse_cjson_into_list(results, "body_markdown", (void *)list_of_question_bodies, num_question_responses);
    
    int c;
    cJSON* list_of_answer_objects[num_question_responses];
    for(c = 0; c < num_question_responses; ++c){
        cJSON *subitem = cJSON_GetArrayItem(results, c); /* Subitem refers to one particular question */
        cJSON* answer = cJSON_GetObjectItem(subitem, "answers");
        list_of_answer_objects[c] = answer;        
    }

    char choice = ' ';
    char context = 'm';
    int current_question_page = 0;
    int current_answer_page = 0;
    while(choice != 'q'){
        if(context == 'm'){
            printf("Title: %s\n\n", list_of_question_titles[current_question_page]);
 
            printf("(d)isplay question body, (s)how answer, (n)ext, (p)revious, (q)uit\n");
            printf(":");
            choice = getchar();

            switch(choice) {
                case 'd' :
                    printf("%s\n", list_of_question_bodies[current_question_page]);
                    break;
                case 's' :
                    context ='s';
                    break;
                case 'n' : 
                    if(current_question_page == num_question_responses - 1){
                        current_question_page = 0;

                    }
                    else{
                        current_question_page++;
 
                    }
                    break;
                case 'p' :
                    if(current_question_page == 0
                    ){
                        current_question_page = num_question_responses -1;
                    }
                    else{
                        current_question_page--;
 
                    }
                    break;
                case 'q' :
                    printf("Good luck!\n");
                    break;
                case '\n':
                    break;
                default:
                    printf("Invalid choice\n");
                    break;
            }
        
        }
        else if(context == 's'){
            cJSON *current_answer_object = list_of_answer_objects[current_question_page];
            int num_answer_responses = cJSON_GetArraySize(current_answer_object);
            char* list_of_answer_bodies[num_answer_responses];
            parse_cjson_into_list(current_answer_object, "body_markdown", (void *)list_of_answer_bodies, num_answer_responses);

            printf("Answer: %s\n", list_of_answer_bodies[current_answer_page]);

            printf("(b)ack, (n)ext, (p)revious, (q)uit\n");
            printf(":");
            choice = getchar();

            switch(choice) {
                case 'b' :
                    context = 'm';
                    break;
                case 'n' : 
                    if(current_answer_page == num_answer_responses - 1){
                        current_answer_page = 0;

                    }
                    else{
                        current_answer_page++;
 
                    }
                    break;
                case 'p' :
                    if(current_answer_page == 0
                    ){
                        current_answer_page = num_answer_responses -1;
                    }
                    else{
                        current_answer_page--;
 
                    }
                    break;
                case 'q' :
                    printf("Good luck!\n");
                    break;
                case '\n':
                    break;
                default:
                    printf("Invalid choice\n");
                    break;
            }
        
        }

    }


    /* Cleanup at the end always */
    free(api_url);
    free(response_from_server.data);
    cJSON_Delete(root);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}