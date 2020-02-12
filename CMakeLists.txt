cmake_minimum_required(VERSION 3.15)
project(smtp_cli C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(smtp_cli
        src/dns.h
        src/email.h
        src/main.h
        src/smtp.h
        src/utils.h
        src/dns.c
        src/email.c
        src/main.c
        src/smtp.c
        src/utils.c)
