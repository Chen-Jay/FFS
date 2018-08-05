//
// Created by jaychen on 18-7-30.
//

#ifndef OS_HOMEWORK_CXX_FILENOTFOUNDEXCEPTION_H
#define OS_HOMEWORK_CXX_FILENOTFOUNDEXCEPTION_H

#endif //OS_HOMEWORK_CXX_FILENOTFOUNDEXCEPTION_H

#include <exception>
using std::exception;

class fileNotFoundException :public exception
{
public:
    fileNotFoundException();
    fileNotFoundException(int exception_code);
    char* what();

private:
    int exception_code;

};