#!/bin/bash

Help()
{
    # Display Help
    echo -e "--------------------------------------------------------"
    echo -e "                       HELP"
    echo -e "--------------------------------------------------------"
    echo
    echo "Скрипт проводит подготовку, трассировку и получение результата трассировки системных вызовов программы"
    echo
    echo "Описание:"
    echo "Скрипт ведет лог своих собственных действий в виде файла get_trace_$date.log"
    echo "Запустите данный скрипт в терминале с одним параметром - названием трассируемой программы"
    echo
    echo "Syntax: syntaxTemplate -h, -n program_name"
    echo
    echo "Usage: $0 [options...]"
    echo
    echo "options:"
    echo "h                 Печать этой Помощи"
    echo
    echo "n program_name    Имя трассируемой программы"
    echo
    echo "Пример:"
    echo "      ./get_trace.sh -n tracing"
    echo
    echo -e "********************************************************************************\n"
}

date=`date '+%Y-%m-%d_%H-%M-%S'`
# echo -e "$date"=
NAME_FILE=$(basename -s .sh $0)
LOG_FILE="${NAME_FILE}_$date.log"

exec &> >(tee ${LOG_FILE})

echo -e "\n********************************************************************************\n"
date
START_TIME=$(date +%s)
echo -e "\n********************************************************************************\n"

echo -e "--------------------------------------------------------"
echo -e "Создание логов скрипта"
echo -e "--------------------------------------------------------\n"
echo -e "Текущий лог файл: LOG_FILE = ${LOG_FILE}"
echo -e
find . -name "${NAME_FILE}_*.log" -mmin +360 -delete
ls -ft ${NAME_FILE}*.log | tail -n +5 | xargs rm -f
echo -e
ls -al ${NAME_FILE}*.log
echo -e

NO_ARGS=0
if [ $# -eq "$NO_ARGS" ]  # Сценарий вызван без аргументов?
then
    Help                   # Если запущен без аргументов - вывести справку
    exit $E_OPTERROR        # и выйти с кодом ошибки
fi

# Get the options
while getopts "hn:" option; do
    case $option in
        h)
            # display Help
            # echo "Действие 1: опция - $option. Номер опции: $OPTIND. Аргумент: $OPTARG"
            Help
            exit 0
            ;;
        n)
            PROG="${OPTARG}"
            if [ "${OPTARG}" == "" ]; then
                echo -e "Отсутствует аргумент для опции -n - название трассируемой программы"
                Help
                exit $E_OPTERROR
            fi
            ;;
        *)
            echo -e
            Help
            exit $E_OPTERROR
            ;;
    esac
done
shift $(($OPTIND - 1))

#########################################################################################
# sudo -i
TRACE="/sys/kernel/debug/tracing"
cd $TRACE
echo -e "Текущий рабочий каталог ${PWD}"
echo 0 > $TRACE/tracing_on                     # Остановить любую существующую трассировку
echo > $TRACE/trace                            # Очистить буфер трассировки
echo function > $TRACE/current_tracer          # Использовать функциональный трассировщик (наиболее надёжный)
echo '__x64_sys_*' > ${TRACE}/set_ftrace_filter  # Фильтровать только системные вызовы


# PID=$(ps -aux | grep ${PROG} | grep -v grep | grep -v "/bin/bash" | sed 's/[^0-9]*//' | cut -c1- | cut -d' ' -f1)
# echo -e "PID = ${PID}"
# echo -e "${PWD}"
# echo "$PID" > ${TRACE}/set_ftrace_pid
# echo "${TRACE}/set_ftrace_pid содержит PID: $(cat ${TRACE}/set_ftrace_pid)"
echo 1 > ${TRACE}/tracing_on
cd -
./${PROG}

echo 0 > ${TRACE}/tracing_on                     # Остановить трассировку
cat ${TRACE}/trace > /tmp/ftrace_output.txt      # Сохранить результаты
# echo > ${TRACE}/set_ftrace_pid                   # Очистить фильтр по PID
echo -e
echo -e "Вывод данных трассировки из /tmp/ftrace_output.txt\n"
head -n 12 /tmp/ftrace_output.txt
cat /tmp/ftrace_output.txt | grep ${PROG}
# rm -f /tmp/ftrace_output.txt
echo -e