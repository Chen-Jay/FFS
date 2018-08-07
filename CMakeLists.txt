cmake_minimum_required(VERSION 3.10)
project(OS_homework[CXX])

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. DIR_SRCS)


add_executable(FFS ${DIR_SRCS} Exceptions/spaceFullException.cpp Exceptions/spaceFullException.h Exceptions/fileNotFoundException.cpp Exceptions/fileNotFoundException.h Exceptions/fileExistException.cpp Exceptions/fileExistException.h )