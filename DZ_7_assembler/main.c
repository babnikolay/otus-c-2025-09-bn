#include <stdio.h>
#include <stdlib.h>

typedef struct Element
{
    long value;
    struct Element *next;
} Element;

void print_int(long value)
{
    printf("%ld ", value);
    fflush(stdout);
}

void add_element(long value, Element **head)
{
    Element *new_elem = malloc(sizeof(Element));
    if (!new_elem)
    {
        abort(); // управление памяти
    }
    new_elem->value = value;
    new_elem->next = *head;
    *head = new_elem;
}

void m(Element *head, void (*process)(long))
{
    if (!head)
        return;
    process(head->value);
    m(head->next, process);
}

void f(Element *head, void (*process)(long))
{
    if (!head)
        return;
    f(head->next, process);
    add_element(head->value, &head);
}

int main()
{
    long data[] = {4, 8, 15, 16, 23, 42};
    size_t data_length = sizeof(data) / sizeof(data[0]);
    Element *list = NULL;

    for (size_t i = 0; i < data_length; ++i)
    {
        add_element(data[i], &list);
    }

    m(list, print_int);
    putchar('\n');

    f(list, print_int);
    putchar('\n');

    // Освобождаем память
    Element *current = list;
    while (current)
    {
        Element *next = current->next;
        free(current);
        current = next;
    }

    return 0;
}
