cmake_minimum_required(VERSION 3.14)
project(MandelbrotSet C)

# --- 1. Проверка компилятора ---
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
if(OpenMP_C_VERSION VERSION_LESS "3.1")
    message(FATAL_ERROR "OpenMP 3.1+ is required.")
endif()

# --- 2. Зависимости (SDL2) ---
include(FetchContent)
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    FetchContent_Declare(SDL2 GIT_REPOSITORY https://github.com GIT_TAG release-2.30.0)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_TARGET SDL2::SDL2-shared)
else()
    set(SDL2_TARGET SDL2::SDL2)
endif()

# --- 3. Поиск файлов (Используем пути относительно CMAKE_CURRENT_SOURCE_DIR) ---
file(GLOB PROG_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/mandelbrot*.c")
file(GLOB ALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set(COMMON_SRCS ${ALL_SRCS})
if(PROG_SRCS)
    list(REMOVE_ITEM COMMON_SRCS ${PROG_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/src/mandel.c")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- 4. Сборка целей ---
set(ALL_TARGET_NAMES "")

foreach(prog_path ${PROG_SRCS})
    get_filename_component(prog_name ${prog_path} NAME_WE)
    
    add_executable(${prog_name} ${prog_path} ${COMMON_SRCS})
    # Добавляем OpenMP сюда, чтобы он был у всех программ:
    target_link_libraries(${prog_name} PRIVATE m OpenMP::OpenMP_C)
    
    if(prog_name STREQUAL "mandelbrot_sdl_mouse")
        target_sources(${prog_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mandel.c")
    endif()

    if(prog_name MATCHES "sdl")
        target_link_libraries(${prog_name} PRIVATE ${SDL2_TARGET} OpenMP::OpenMP_C)
    endif()

    # --- МАГИЯ: Перенос в dist и удаление из build сразу после сборки ---
    add_custom_command(TARGET ${prog_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/dist/bin"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${prog_name}> "${CMAKE_SOURCE_DIR}/dist/bin/"
        COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE:${prog_name}>
        COMMENT "Moving ${prog_name} to dist/bin and cleaning build directory"
    )

    list(APPEND ALL_TARGET_NAMES ${prog_name})
endforeach()

# --- 5. Финальный статус и сообщения (с проверкой на наличие целей) ---
if(ALL_TARGET_NAMES)
    add_custom_target(finish_status ALL
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan --bold "\n--- Сборка УСПЕШНА: " --reset
        COMMAND date +"%H:%M:%S"
        COMMAND echo ""
    )
    # Исправлено: передаем список целей как аргументы
    add_dependencies(finish_status ${ALL_TARGET_NAMES})
else()
    message(WARNING "No source files matching 'src/mandelbrot*.c' were found!")
endif()

# Установка SDL2 если собрана локально
if(TARGET SDL2-shared)
    install(TARGETS SDL2-shared RUNTIME DESTINATION bin LIBRARY DESTINATION bin)
endif()
