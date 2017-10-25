void draw_duck();
void show_menu();
char *get_input();
void parse_cjson_into_list(cJSON *items, char *field, void* list_of_items[], int num_responses);
char *build_search_request(char *query);