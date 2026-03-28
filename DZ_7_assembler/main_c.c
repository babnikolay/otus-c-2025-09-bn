#include <stdio.h>
#include <stdlib.h>

struct node {
    long value;
    struct node *next;
};

long data[] = {4, 8, 15, 16, 23, 42};
int data_length = sizeof(data) / sizeof(long);

void print_int(long val) {
    printf("%ld ", val);
    fflush(stdout);
}

long p(long val) {
    return val & 1;
}

struct node* add_element(long val, struct node* head) {
    struct node* new_node = malloc(sizeof(struct node));
    if (!new_node) abort();
    new_node->value = val;
    new_node->next = head;
    return new_node;
}

// Рекурсивная функция m (как в оригинале)
void m(struct node* head, void (*func)(long)) {
    if (!head) return;
    func(head->value);
    m(head->next, func);
}

// Рекурсивная функция f (фильтрация)
struct node* f(struct node* head, struct node* acc, long (*predicate)(long)) {
    if (!head) return acc;
    if (predicate(head->value)) {
        acc = add_element(head->value, acc);
    }
    return f(head->next, acc, predicate);
}

// Новая функция для освобождения памяти
void free_list(struct node* head) {
    while (head) {
        struct node* temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    struct node* list = NULL;

    // Сборка списка
    for (int i = data_length - 1; i >= 0; i--) {
        list = add_element(data[i], list);
    }

    m(list, print_int);
    puts("");

    // Создание нового списка из нечетных элементов
    struct node* filtered_list = f(list, NULL, p);

    m(filtered_list, print_int);
    puts("");

    // Освобождаем оба списка
    free_list(list);
    free_list(filtered_list);

    return 0;
}
