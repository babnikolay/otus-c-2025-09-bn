#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

// Функция для завершения работы с выводом ошибки
static void finish_with_error(PGconn *conn, PGresult *res) {
    if (res) fprintf(stderr, "Result error: %s\n", PQresultErrorMessage(res));
    if (conn) fprintf(stderr, "DB error: %s\n", PQerrorMessage(conn));
    if (res) PQclear(res);
    if (conn) PQfinish(conn);
    exit(EXIT_FAILURE);
}

// Функция загрузки данных из CSV в таблицу
void load_csv(PGconn *conn, const char *table, const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror("Failed to open CSV file");
        return;
    }

    char query[256];
    // Используем STDIN для передачи данных напрямую через клиентское приложение
    snprintf(query, sizeof(query), "COPY %s FROM STDIN WITH (FORMAT csv, HEADER true);", table);

    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COPY_IN) {
        fclose(f);
        finish_with_error(conn, res);
    }
    PQclear(res);

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f)) {
        if (PQputCopyData(conn, buffer, strlen(buffer)) != 1) {
            fprintf(stderr, "Error sending data to server\n");
        }
    }

    if (PQputCopyEnd(conn, NULL) != 1) {
        fprintf(stderr, "Error finalizing COPY\n");
    }

    res = PQgetResult(conn);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fclose(f);
        finish_with_error(conn, res);
    }
    PQclear(res);
    fclose(f);
    printf("Successfully loaded data from '%s' into table '%s'.\n", filepath, table);
}

int main(int argc, char *argv[]) {
    // 1. Проверка аргументов командной строки
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <dbname> <tablename> <columnname> [csv_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dbname = argv[1];
    const char *table = argv[2];
    const char *column = argv[3];
    const char *csv_path = (argc > 4) ? argv[4] : NULL;

    // 2. Подключение к БД
    char conninfo[256];
    snprintf(conninfo, sizeof(conninfo), "host=localhost port=5432 user=postgres password=postgres dbname=%s", dbname);

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return EXIT_FAILURE;
    }

    // 3. Автоматическое создание таблицы (на примере структуры Oscar)
    char create_query[512];
    snprintf(create_query, sizeof(create_query), 
             "CREATE TABLE IF NOT EXISTS %s ("
             "id INTEGER, year INTEGER, age INTEGER, name TEXT, movie TEXT);", table);

    PGresult *res = PQexec(conn, create_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) finish_with_error(conn, res);
    PQclear(res);

    // 4. Загрузка данных из CSV, если файл указан
    if (csv_path) {
        load_csv(conn, table, csv_path);
    }

    // 5. Проверка: является ли колонка числовой
    char check_query[512];
    snprintf(check_query, sizeof(check_query), 
             "SELECT data_type FROM information_schema.columns "
             "WHERE table_name = '%s' AND column_name = '%s';", table, column);

    res = PQexec(conn, check_query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Error: Column '%s' or table '%s' not found.\n", column, table);
        finish_with_error(conn, res);
    }

    char *data_type = PQgetvalue(res, 0, 0);
    if (!strstr(data_type, "int") && !strstr(data_type, "double") && 
        !strstr(data_type, "numeric") && !strstr(data_type, "real")) {
        fprintf(stderr, "Error: Column '%s' is non-numeric (type: %s). Cannot calculate stats.\n", column, data_type);
        finish_with_error(conn, res);
    }
    PQclear(res);

    // 6. Расчет статистических параметров
    char stats_query[1024];
    snprintf(stats_query, sizeof(stats_query),
             "SELECT "
             "AVG(%s::double precision), "
             "MAX(%s), "
             "MIN(%s), "
             "VARIANCE(%s::double precision), "
             "SUM(%s) "
             "FROM %s;", 
             column, column, column, column, column, table);

    res = PQexec(conn, stats_query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) finish_with_error(conn, res);

    // 7. Вывод результатов
    if (PQntuples(res) > 0 && *PQgetvalue(res, 0, 0) != '\0') {
        printf("\n--- Statistics for %s.%s ---\n", table, column);
        printf("Average:  %s\n", PQgetvalue(res, 0, 0));
        printf("Maximum:  %s\n", PQgetvalue(res, 0, 1));
        printf("Minimum:  %s\n", PQgetvalue(res, 0, 2));
        printf("Variance: %s\n", PQgetvalue(res, 0, 3));
        printf("Sum:      %s\n", PQgetvalue(res, 0, 4));
        printf("------------------------------\n");
    } else {
        printf("Table is empty. No statistics to show.\n");
    }

    PQclear(res);
    PQfinish(conn);
    return EXIT_SUCCESS;
}
