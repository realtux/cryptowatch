#include <ctype.h>
#include <locale.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <curl/curl.h>
#include <json.h>

#include "cryptowatch.h"

static int coin_count = 1;
static double btc_price = 0;

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");

    if (argc < 2) {
        menu();
        return 1;
    }

    int i;

    // main storage for the coin data
    coin *coins = malloc(sizeof(coin));

    // prep first coin
    coins[0].symbol[0] = '\0';

    // parse out the coins and add
    char *arg1 = argv[1];

    int symbols_len = strlen(argv[1]);
    for (i = 0; i < symbols_len; ++i) {
        if (arg1[i] == ' ') continue;

        // new coin
        if (arg1[i] == ',' && i + 1 != symbols_len) {
            ++coin_count;
            coins = realloc(coins, sizeof(coin) * coin_count);
            coins[coin_count - 1].symbol[0] = '\0';
            continue;
        }

        int symbol_len = strlen(coins[coin_count - 1].symbol);

        coins[coin_count - 1].symbol[symbol_len] = arg1[i];
        coins[coin_count - 1].symbol[symbol_len + 1] = '\0';
        coins[coin_count - 1].price = 0;
        coins[coin_count - 1].change = 0;
        coins[coin_count - 1].color = 0;
    }

    // create threads for ui and all coins
    pthread_t ui_thread;
    pthread_t btc_thread;
    pthread_t *coin_threads = malloc(sizeof(pthread_t) * coin_count);

    pthread_create(&ui_thread, NULL, update_ui, coins);
    pthread_create(&btc_thread, NULL, watch_btc, NULL);

    for (i = 0; i < coin_count; ++i) {
        pthread_create(&coin_threads[i], NULL, watch_coin, &coins[i]);
    }

    // basically block the program from exiting
    pthread_join(ui_thread, NULL);
}

void menu(void) {
    printf(
        "cryptowatch 0.0.1 by b\n"
        "usage: cryptowatch [symbols]\n"
        "examples:\n"
        "    - cryptowatch zec\n"
        "    - cryptowatch btc,eth,ltc\n"
    );
}

void *update_ui(void *arg) {
    coin *coins = (coin *) arg;

    for (;;) {
        printf("\033[2J\033[1;1H");
        printf("\ncryptowatch 0.0.1 by b:\n\n");
        for (int i = 0; i < coin_count; ++i) {
            if (coins[i].color == 0) {
                printf("%-8s $%9.2f     %.2f%%\n",
                    coins[i].symbol,
                    coins[i].price,
                    coins[i].change
                );
            } else if (coins[i].color == 1) {
                printf("%-8s $%9.2f    \033[32;1m%s%.2f%%\033[0m\n",
                    coins[i].symbol,
                    coins[i].price,
                    coins[i].color == 1 ? "+" : "-",
                    coins[i].change
                );
            } else {
                printf("%-8s $%9.2f    \033[31;1m%s%.2f%%\033[0m\n",
                    coins[i].symbol,
                    coins[i].price,
                    coins[i].color == 1 ? "+" : "-",
                    coins[i].change
                );
            }
        }
        printf("\nprices based on btc value of: $%.2f\n\n", btc_price);
        sleep(1);
    }

    return NULL;
}

void *watch_btc(void *arg) {
    (void)arg;

    const char *market = "usdt-btc";

    for (;;) {
        char *response = price_data(market);

        if (response == NULL) {
            sleep(60);
            free(response);
            continue;
        }

        json_object *payload = json_tokener_parse(response);
        json_object *result = json_get_array_by_idx(payload, "result", 0);

        btc_price = json_get_double(result, "Last");

        free(response);

        sleep(60);
    }

    return NULL;
}

void *watch_coin(void *arg) {
    coin *watched_coin = (coin *) arg;

    char market[32];

    if (strcmp(watched_coin->symbol, "btc") == 0) {
        strcpy(market, "usdt-");
    } else {
        strcpy(market, "btc-");
    }

    strcat(market, watched_coin->symbol);

    // give the btc price a sec to update
    sleep(3);

    for (;;) {
        char *response = price_data(market);

        if (response == NULL) {
            sleep(10);
            free(response);
            continue;
        }

        json_object *payload = json_tokener_parse(response);

        // check for bad coin
        char *message = json_get_string(payload, "message");

        // if a coin is watched but doesn't exist on trex, just exit the thread
        if (message == NULL || strcmp(message, "INVALID_MARKET") == 0) {
            free(message);
            free(response);
            return NULL;
        }

        json_object *result = json_get_array_by_idx(payload, "result", 0);

        double last = json_get_double(result, "Last");
        double prev = json_get_double(result, "PrevDay");

        if (strcmp(market, "usdt-btc") == 0) {
            watched_coin->price = last;
        } else {
            watched_coin->price = last * btc_price;
        }

        if (last >= prev) {
            watched_coin->change = 100 - ((prev / last) * 100);
            watched_coin->color = 1;
        } else {
            watched_coin->change = 100 - ((last / prev) * 100);
            watched_coin->color = 2;
        }

        free(response);

        sleep(10);
    }

    return NULL;
}

size_t write_result(void *ptr, size_t size, size_t nmemb, struct curl_data *data) {
    size_t realsize = size * nmemb;
    struct curl_data *mem = (struct curl_data *)data;

    mem->contents = realloc(mem->contents, mem->len + realsize + 120);

    if (mem->contents) {
        memcpy(&(mem->contents[mem->len]), ptr, realsize);
        mem->len += realsize;
        mem->contents[mem->len] = 0;
    }

    return realsize;
}

json_object *json_get_object(json_object *jobj, const char *object_name) {
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);

        if (type == json_type_object && strcmp(key, object_name) == 0)
            return val;
    }

    return NULL;
}

json_object *json_get_array(json_object *jobj, const char *array_name) {
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);

        if (type == json_type_array && strcmp(key, array_name) == 0)
            return val;
    }

    return NULL;
}

json_object *json_get_array_by_idx(json_object *jobj, const char *array_name, int idx) {
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);

        if (type == json_type_array && strcmp(key, array_name) == 0)
            return json_object_array_get_idx(val, idx);
    }

    return NULL;
}

char *json_get_string(json_object *jobj, const char *string_name) {
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);

        if (type == json_type_string && strcmp(key, string_name) == 0)
            return (char *)json_object_get_string(val);
    }

    return NULL;
}

double json_get_double(json_object *jobj, const char *string_name) {
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);

        if (type == json_type_double && strcmp(key, string_name) == 0)
            return json_object_get_double(val);
    }

    return 0;
}

char *price_data(const char *market) {
    CURL *curl;
    CURLcode res_code;

    curl = curl_easy_init();

    if (curl == NULL) {
        printf("trouble initing curl\n");
        exit(1);
    }

    struct curl_data s;
    s.contents = NULL;
    s.len = 0;

    char api_base[128] = "https://bittrex.com/api/v1.1/public/getmarketsummary?market=";

    strcat(api_base, market);

    curl_easy_setopt(curl, CURLOPT_URL, api_base);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_result);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &s);

    res_code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res_code != 0) return NULL;

    return s.contents;
}
