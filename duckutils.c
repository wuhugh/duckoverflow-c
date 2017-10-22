#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "duckutils.h"

/* Draw the duck */
void draw_duck(){
    const char *duck[3];
    duck[0] = "<(o )___\n";
    duck[1] = " ( ._> /\n";
    duck[2] = " `---'\n";
    
    for(int i = 0; i < 3; ++i){
        printf("%s", duck[i]);
    }
}

/* Replaces space characters with the plus sign to make a proper URL */
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

char *build_request(char *query){
    char *base_url = "https://api.stackexchange.com/2.2/search/advanced?order=desc&sort=votes&site=stackoverflow&answers=1&q=";
    /*
    It's possible to make the program more flexible by allowing users to change these settings
    but in this case it wouldn't do much and it takes up memory
    char *order = "order=desc";
    char *sort = "&sort=votes";
    char *site= "&site=stackoverflow";
    char *answers = "&answers=1"
    */

    query = replace_space(query);
    char *full_api_url = malloc(strlen(base_url) + strlen(query) + 1);
    strcpy(full_api_url, base_url);
    strcat(full_api_url, query);

    free(query);
    return full_api_url;
}