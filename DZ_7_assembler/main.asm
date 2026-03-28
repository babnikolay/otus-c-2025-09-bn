    bits 64
    extern malloc, free, puts, printf, fflush, abort
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text

;;; Процедура освобождения памяти: free_list
;;; free_list: Итеративно проходит по списку и вызывает free для каждого узла. 
;;; Это безопаснее рекурсии, так как не переполнит стек на длинных списках
;;; rdi - адрес головы списка
free_list:
    test rdi, rdi
    jz .exit
    push rbx
.loop:
    test rdi, rdi
    jz .done
    mov rbx, [rdi + 8]   ; сохраняем указатель на следующий элемент
    call free            ; rdi уже содержит текущий элемент
    mov rdi, rbx         ; переходим к следующему
    jmp .loop
.done:
    pop rbx
.exit:
    ret

;;; Печать числа
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf
    xor rdi, rdi
    call fflush
    mov rsp, rbp
    pop rbp
    ret

;;; Проверка на нечетность
p:
    mov rax, rdi
    and rax, 1
    ret

;;; Добавление элемента (создает новый узел)
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 16
    mov r14, rdi
    mov rbx, rsi
    mov rdi, 16
    call malloc
    test rax, rax
    jz abort
    mov [rax], r14
    mov [rax + 8], rbx
    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc (Оптимизирована: Итеративный цикл вместо рекурсии)
m:
    test rdi, rdi       ; Если список пуст (NULL)
    jz .out             ; Выходим
    push rbx            ; Сохраняем callee-saved регистры
    push r12            ; rbx - узел, r12 - функция
    mov rbx, rdi        ; текущий узел
    mov r12, rsi        ; функция обработки

.loop:
    ; 1. Обработка текущего узла
    mov rdi, [rbx]      ; Значение узла в rdi
    call r12            ; Вызываем функцию (например, print_int)

    ; 2. Переход к следующему узлу
    mov rbx, [rbx + 8]  ; rbx = rbx->next
    
    ; 3. Проверка: есть ли следующий элемент?
    test rbx, rbx
    jnz .loop           ; Если есть, прыгаем в начало цикла (это и есть "хвост")

    pop r12             ; Если элементов нет, восстанавливаем стек
    pop rbx
.out:
    ret


;;; f proc (Итеративная версия: фильтрация через "указатель на указатель")
;;; rdi - head (исходный список)
;;; rdx - predicate (функция p)
f:
    push rbx
    push r12
    push r13
    push r14

    mov rbx, rdi        ; rbx = текущий узел исходного списка
    mov r12, rdx        ; r12 = предикат (p)

    sub rsp, 8           ; локальная переменная для головы нового списка
    mov qword [rsp], 0   ; head = NULL
    mov r14, rsp         ; r14 = адрес, куда писать адрес следующего узла (указатель на указатель)

.loop:
    test rbx, rbx
    jz .done

    ; Вызываем предикат p(node->value)
    mov rdi, [rbx]
    call r12
    
    test rax, rax
    jz .next_node       ; Если 0, пропускаем

    ; Если предикат вернул 1, создаем новый элемент
    mov rdi, [rbx]      ; value
    xor rsi, rsi        ; next = NULL (пока что)
    call add_element    ; rax = новый узел
    
    mov [r14], rax      ; Записываем адрес нового узла в "next" предыдущего (или в голову)
    lea r14, [rax + 8]   ; теперь r14 указывает на поле 'next' нового узла

.next_node:
    mov rbx, [rbx + 8]  ; Переходим к следующему элементу исходного списка
    jmp .loop

.done:
    pop rax             ; Забираем адрес головы нового списка со стека
    pop r14
    pop r13
    pop r12
    pop rbx
    ret


;;; Добавлено сохранение результата функции f в регистр r12 (callee-saved), чтобы он не потерялся
;;; В конце добавлены вызовы free_list для обоих списков
main:
    push rbx
    push r12                ; используем r12 для хранения второго списка

    xor rax, rax            ; tail = NULL
    mov rbx, data_length
adding_loop:
    mov rdi, [data - 8 + rbx * 8]
    mov rsi, rax
    call add_element
    dec rbx
    jnz adding_loop

    mov rbx, rax         ; rbx = исходный список

    mov rdi, rbx
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

    mov rdi, rbx        ; Исходный список
    mov rdx, p          ; Предикат
    call f              ; Результат сразу в rax
    mov r12, rax        ; r12 = отфильтрованный список

    mov rdi, r12
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

    ; Очистка  памяти
    mov rdi, rbx
    call free_list
    mov rdi, r12
    call free_list

    pop r12
    pop rbx
    xor rax, rax
    ret

;;; Секция для предотвращения предупреждения линковщика об исполняемом стеке
section .note.GNU-stack noalloc noexec nowrite progbits
