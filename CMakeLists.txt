cmake_minimum_required(VERSION 3.30)
project(simpleREST)

set(CMAKE_CXX_STANDARD 20)

# Ищем библиотеки, установленные через vcpkg
find_package(Crow CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

add_executable(simpleREST main.cpp)

# Подключаем библиотеки
target_link_libraries(simpleREST PRIVATE Crow::Crow SQLiteCpp)
