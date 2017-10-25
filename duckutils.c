#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "duckutils.h"

/* Draw the duck */
void draw_duck(){
    printf("Tell me what's wrong\n");
    printf("                     <(o )___\n");
    printf("                      ( ._> /\n");
    printf("                       `---'\n");
    printf("You: ");
    
    return;
}

/* Show choices to the user */
void show_menu(){
    return;
}

/** 
 * @brief Reads input from the user until the newline is hit or EOF
 *
 * Shamelessly stolen from Stackoverflow - thanks hackks 
 * 
 * @return The user input as a char pointer
*/
char *get_input(){
    /* Create the string */
    char *input = malloc(1);
    char c = 0;
    int i = 0;

    /* Read from the input buffer until a new line is hit (enter) */
    while((c = getchar()) != '\n' && c != EOF){
        input[i++] = c; /* Concatenate the string */
        input = realloc(input, i + 1); /* Add room for more */
    }
    input[i] = '\0'; /* Zero terminate */
    return input;
}

/**
 * @brief Replaces space characters with the plus sign to make a proper URL
 * 
 * URLs cannot have spaces so we replace them with plus signs.
 * Technically the plus sign is only valid for queries and %20 
 * should be used in other places
 * 
 * @param char* full_text: The string we want to operate one
 * 
 * @return A string that contains the whitespace replaced with plus signs
*/
char *replace_space(char *full_text){
    char *result = malloc(strlen(full_text) * sizeof(char));
    
    /* Because we have a pointer to a char, we can't change the values directly
    or else we'll get a seg fault. A copy is made and returned here but there might
    be a better way... */
    int i;
    char *p = full_text;
    for(i = 0; i < strlen(full_text); ++i){
        if(*p == ' '){
            result[i] = '+';
        }
        else{
            result[i] = *p;
        }
        p++;
    }

    return result;
}

/**
 * @brief Parses a cJSON object and saves the value of theh specified field into an array
 * 
 * @param cJSON* items: The object holding the data we want
 * @param char *field: The data field of the object we want
 * @param char* list_of_items[]: An array of strings that contain each contain a field value
 * @param int num_responses: The number of item in the list_of_items
 *   @note that if the JSON object a different numer data, only num_responses is saved
 *   This may throw an error if num responses is greater than the number of actual data 
 * 
 * @return Nothing, but the list_of_items is altered
*/ 
void parse_cjson_into_list(cJSON* items, char* field, void* list_of_items[], int num_responses){
    int i;
    for(i = 0; i < num_responses; ++i){
        cJSON *subitem = cJSON_GetArrayItem(items, i);    
        cJSON *data = cJSON_GetObjectItem(subitem, field);
        char* rendered = cJSON_Print(data);

        list_of_items[i] = rendered;
    }
    return;
}

/* Given a text query, this constructus the URL to get a list of titles, questions ids, etc
matched by that query  */
char *build_search_request(char *query){
    char *base_url = "https://api.stackexchange.com/2.2/search/advanced?order=desc&sort=votes&site=stackoverflow&answers=1&filter=sh-X*VEiDQuCRN)Ihmxav)p(5ge10gJpN5y&q=";
    
    /*
    It's possible to make the program more flexible by allowing users to change these settings
    but in this case it wouldn't do much and it takes up memory. These are a pretty good default. 
    char *order = "order=desc"; Order by highest first
    char *sort = "&sort=votes"; Get the highest voted first
    char *site= "&site=stackoverflow"; Check only Stackoverflow
    char *answers = "&answers=1" Returns responses with at least 1 answer
    char *filters = "!JDwxy(MfhiWlv9J-ntatKerGl93aEpHd1e" Custom filter to exclude stuff like avatar URLs
    */

    /* Construct the API URL */
    query = replace_space(query);
    char *full_api_url = malloc(strlen(base_url) + strlen(query) + 1);
    strcpy(full_api_url, base_url);
    strcat(full_api_url, query);

    free(query);
    return full_api_url;
}
