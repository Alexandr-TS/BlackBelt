# CMakeList.txt: проект CMake для Solutions; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Solutions   "test_runner.h"  "apply_to_many_2.cpp")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
