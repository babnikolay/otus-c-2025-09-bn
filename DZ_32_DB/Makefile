CC = gcc
TARGET = db_postgres
SRC = db_postgres.c
SRC_DIR = src
BUILD_DIR = build
CSV_FILE = oscars.csv
DB_NAME = postgres
TABLE_NAME = oscar
export PGPASSWORD = postgres

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 $(shell pg_config --includedir | sed 's/^/-I/')
LDFLAGS = $(shell pg_config --libdir | sed 's/^/-L/') -lpq

all: $(BUILD_DIR) $(TARGET)

# Создание директории сборки
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Сборка основного приложения
$(TARGET): $(SRC_DIR)/$(SRC)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(SRC_DIR)/$(SRC) $(LDFLAGS)

# Сборка с отладочными символами и флагом _DEBUG
debug: $(BUILD_DIR)
	$(CC) -g -O0 $(CFLAGS) -D_DEBUG -o $(BUILD_DIR)/$(TARGET)_debug $(SRC) $(LDFLAGS)

# Команда для удаления таблицы перед тестом
reset_db:
	@echo "Dropping table $(TABLE_NAME) in database $(DB_NAME)..."
	@psql -h localhost -U postgres -d $(DB_NAME) -c "DROP TABLE IF EXISTS $(TABLE_NAME);"

# Создание тестового CSV-файла
testdata:
	@echo "id,year,age,name,movie" > $(CSV_FILE)
	@echo "1,1928,22,Janet Gaynor,7th Heaven" >> $(CSV_FILE)
	@echo "2,1929,35,Mary Pickford,Coquette" >> $(CSV_FILE)
	@echo "3,1930,30,Norma Shearer,The Divorcee" >> $(CSV_FILE)
	@echo "4,1931,62,Marie Dressler,Min and Bill" >> $(CSV_FILE)
	@echo "5,1932,32,Helen Hayes,The Sin of Madelon Claudet" >> $(CSV_FILE)
	@echo "89,2016,26,Brie Larson,Room" >> $(CSV_FILE)
	@echo "Test data generated in $(CSV_FILE)"

# Запуск полного цикла: сборка, генерация данных и выполнение
test: all reset_db  testdata
	./$(BUILD_DIR)/$(TARGET) $(DB_NAME) $(TABLE_NAME) age $(CSV_FILE)

clean:
	rm -rf $(BUILD_DIR) $(CSV_FILE)

.PHONY: all clean debug testdata test reset_db 
