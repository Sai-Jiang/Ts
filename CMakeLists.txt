cmake_minimum_required(VERSION 3.7)
project(Ts)

set(CMAKE_C_STANDARD 99)

add_executable(TCP_Server_Ts TCP_Server_Ts.c)
add_executable(TCP_Client_Ts TCP_Client_Ts.c)

add_executable(UDP_Server_Ts UDP_Server_Ts.c)
add_executable(UDP_Client_Ts UDP_Client_Ts.c)