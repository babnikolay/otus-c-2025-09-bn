#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <limits.h>

#define MAX_LINE 16384
#define HASH_SIZE 1048576

typedef struct Node {
    char *key;
    long long value;
    struct Node *next;
} Node;

typedef struct {
    Node *table[HASH_SIZE];
    pthread_mutex_t lock;
} HashTable;

HashTable url_stats, ref_stats;
long long global_total_bytes = 0;
pthread_mutex_t bytes_lock = PTHREAD_MUTEX_INITIALIZER;

unsigned int hash(const char *str) {
    unsigned int h = 5381;
    while (*str) h = ((h << 5) + h) + (unsigned char)*str++;
    return h % HASH_SIZE;
}

void add_stat(HashTable *ht, const char *key, long long val) {
    if (!key || *key == '\0') return;
    unsigned int idx = hash(key);
    pthread_mutex_lock(&ht->lock);
    Node *curr = ht->table[idx];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            curr->value += val;
            pthread_mutex_unlock(&ht->lock);
            return;
        }
        curr = curr->next;
    }
    Node *new_node = malloc(sizeof(Node));
    new_node->key = strdup(key);
    new_node->value = val;
    new_node->next = ht->table[idx];
    ht->table[idx] = new_node;
    pthread_mutex_unlock(&ht->lock);
}

void parse_combined(char *line) {
    char *first_quote = strchr(line, '\"');
    if (!first_quote) return;
    char *second_quote = strchr(first_quote + 1, '\"');
    if (!second_quote) return;

    // 1. URL
    char *url = NULL;
    size_t req_len = (size_t)(second_quote - first_quote - 1);
    if (req_len > 0) {
        char *req_tmp = strndup(first_quote + 1, req_len);
        char *s_ptr;
        strtok_r(req_tmp, " ", &s_ptr); // Method
        char *u = strtok_r(NULL, " ", &s_ptr);
        if (u) url = strdup(u);
        free(req_tmp);
    }

    // 2. Bytes
    char *post_req = second_quote + 1;
    char *endptr;
    strtol(post_req, &endptr, 10); // Status
    long long b = strtoll(endptr, &endptr, 10);

    // 3. Referer
    char *referer = NULL;
    char *ref_start = strchr(second_quote + 1, '\"');
    if (ref_start) {
        char *ref_end = strchr(ref_start + 1, '\"');
        if (ref_end) {
            size_t r_len = (size_t)(ref_end - ref_start - 1);
            if (r_len > 0) referer = strndup(ref_start + 1, r_len);
        }
    }

    if (url) { add_stat(&url_stats, url, b); free(url); }
    if (referer) {
        if (strcmp(referer, "-") != 0) add_stat(&ref_stats, referer, 1);
        free(referer);
    }
    if (b > 0) {
        pthread_mutex_lock(&bytes_lock);
        global_total_bytes += b;
        pthread_mutex_unlock(&bytes_lock);
    }
}

typedef struct { char **paths; int count; } ThreadData;

void* worker(void* arg) {
    ThreadData *td = (ThreadData*)arg;
    for (int i = 0; i < td->count; i++) {
        FILE *f = fopen(td->paths[i], "r");
        if (!f) continue;
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), f)) parse_combined(line);
        fclose(f);
    }
    return NULL;
}

int compare_nodes(const void *a, const void *b) {
    long long v_a = (*(Node**)a)->value, v_b = (*(Node**)b)->value;
    return (v_b > v_a) - (v_b < v_a);
}

void print_top(HashTable *ht, const char *label, const char *suffix) {
    size_t cap = 1000000, total = 0;
    Node **list = malloc(cap * sizeof(Node*));
    for (int i = 0; i < HASH_SIZE; i++) {
        for (Node *n = ht->table[i]; n; n = n->next) {
            if (total >= cap) { cap *= 2; list = realloc(list, cap * sizeof(Node*)); }
            list[total++] = n;
        }
    }
    if (total > 0) {
        qsort(list, total, sizeof(Node*), compare_nodes);
        printf("\n--- ТОП 10 %s ---\n", label);
        for (size_t i = 0; i < (total < 10 ? total : 10); i++)
            printf("%12lld %s \t %s\n", list[i]->value, suffix, list[i]->key);
    }
    free(list);
}

void free_hash_table(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Node *curr = ht->table[i];
        while (curr) {
            Node *temp = curr;
            curr = curr->next;
            free(temp->key);
            free(temp);
        }
    }
    pthread_mutex_destroy(&ht->lock);
}

int main(int argc, char *argv[]) {
    if (argc < 3) return 1;
    int num_t = atoi(argv[2]);
    DIR *d = opendir(argv[1]);
    if (!d) return 1;

    pthread_mutex_init(&url_stats.lock, NULL);
    pthread_mutex_init(&ref_stats.lock, NULL);

    ThreadData *tds = calloc((size_t)num_t, sizeof(ThreadData));
    for(int i=0; i<num_t; i++) tds[i].paths = malloc(sizeof(char*) * 10000);

    struct dirent *en;
    int f_idx = 0;
    while ((en = readdir(d))) {
        if (en->d_type == DT_REG) {
            tds[f_idx % num_t].paths[tds[f_idx % num_t].count] = malloc(PATH_MAX);
            snprintf(tds[f_idx % num_t].paths[tds[f_idx % num_t].count++], PATH_MAX, "%s/%s", argv[1], en->d_name);
            f_idx++;
        }
    }
    closedir(d);

    if (f_idx > 0) {
        pthread_t *tids = malloc((size_t)num_t * sizeof(pthread_t));
        for (int i = 0; i < num_t; i++) pthread_create(&tids[i], NULL, worker, &tds[i]);
        for (int i = 0; i < num_t; i++) pthread_join(tids[i], NULL);
        
        printf("Total bytes transferred: %lld\n", global_total_bytes);
        print_top(&url_stats, "URL по трафику", "байт");
        print_top(&ref_stats, "Referer по частоте", "раз ");
        free(tids);
    } else {
        printf("Directory is empty.\n");
    }

    free_hash_table(&url_stats);
    free_hash_table(&ref_stats);
    for(int i=0; i<num_t; i++) {
        for(int j=0; j<tds[i].count; j++) free(tds[i].paths[j]);
        free(tds[i].paths);
    }
    free(tds);
    pthread_mutex_destroy(&bytes_lock);

    return 0;
}
