//
// Created by jaychen on 18-7-30.
//

#ifndef OS_HOMEWORK_CXX_FILEEXISTEXCEPTION_H
#define OS_HOMEWORK_CXX_FILEEXISTEXCEPTION_H

#endif //OS_HOMEWORK_CXX_FILEEXISTEXCEPTION_H

#include <exception>
using std::exception;

class fileExistException:public exception
{
public:
    char *what();
};