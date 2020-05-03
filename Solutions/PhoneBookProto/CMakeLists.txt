cmake_minimum_required (VERSION 3.8)

find_package( Protobuf REQUIRED )

include_directories(${Protobuf_INCLUDE_DIRS}) # Добавляем в include path пути к библиотеке protobuf
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # В ${CMAKE_CURRENT_BINARY_DIR} будут файлы, сгенерированные компилятором protoс, путь к ним надо добавить в include path

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS contact.proto) # Здесь надо перечислить все ваши .proto-файлы
# add_executable(main main.cpp ${PROTO_SRCS} ${PROTO_HDRS}) # Здесь надо перечислить все ваши .cpp-файлы, в том числе и сгенерированные protoc'ом
add_executable (CourseraBlackBelt "main.cpp" "iterator_range.h" "test_runner.h" "phone_book.h" ${PROTO_SRCS} ${PROTO_HDRS}) 
target_link_libraries(CourseraBlackBelt ${Protobuf_LIBRARIES}) # компонуем наш исполняемый файл с библиотекой libprotobuf 

