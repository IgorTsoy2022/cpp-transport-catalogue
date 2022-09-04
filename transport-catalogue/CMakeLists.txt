cmake_minimum_required (VERSION 3.10)

project ("TransportCatalogue")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH /protobuf/package)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# Команда вызова protoc. 
# Ей переданы названия переменных, в которые будут сохранены 
# списки сгенерированных файлов, а также сам proto-файл.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
                      transport_catalogue.proto
                      svg.proto
                      map_renderer.proto
                      transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES
    domain.h domain.cpp geo.h geo.cpp graph.h json.h json.cpp
    json_builder.h json_builder.cpp json_reader.h json_reader.cpp
    map_renderer.h map_renderer.cpp ranges.h
    request_handler.h request_handler.cpp router.h
    svg.h svg.cpp serialization.h serialization.cpp
    transport_catalogue.h transport_catalogue.cpp
    transport_router.h transport_router.cpp
    main.cpp)

# добавляем цель - transport_catalogue
add_executable(transport_catalogue
               ${PROTO_SRCS} ${PROTO_HDRS}
               ${TRANSPORT_CATALOGUE_FILES})

# find_package определила переменную Protobuf_INCLUDE_DIRS,
# которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда
# protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Также find_package определила Protobuf_LIBRARY и Protobuf_LIBRARY_DEBUG.
# Исправим некорректное значение Protobuf_LIBRARY_DEBUG операцией замены:
string(REPLACE "protobuf.lib" "protobufd.lib"
       "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a"
       "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue
 "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>"
 Threads::Threads)