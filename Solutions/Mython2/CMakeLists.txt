# CMakeList.txt: проект CMake для Solutions; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Solutions "lexer.cpp" "lexer_test.cpp" 
	"comparators.cpp" "mython.cpp" "object.cpp" "object_holder.cpp" "object_holder_test.cpp" 
	"object_test.cpp" "object_test.cpp" "parse.cpp" "parse_test.cpp" "statement.cpp" "statement_test.cpp"
	"lexer.h" "test_runner.h" "comparators.h" "object.h" "object_holder.h" "parse.h" "statement.h"
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
