#ifndef CRYPTOWATCH_H
#define CRYPTOWATCH_H

typedef struct {
    char symbol[16];
    double price;
    double change;
    int color; // 1 - green, 2 - red
} coin;

struct curl_data {
    char *contents;
    size_t len;
};

enum json_type type;

void menu(void);
void *update_ui(void*);
void *watch_btc(void*);
void *watch_coin(void*);
size_t write_result(void*, size_t, size_t, struct curl_data*);
json_object *json_get_object(json_object*, const char*);
json_object *json_get_array(json_object*, const char*);
json_object *json_get_array_by_idx(json_object*, const char*, int);
char *json_get_string(json_object*, const char*);
double json_get_double(json_object*, const char*);
char *price_data(const char*);

#endif
