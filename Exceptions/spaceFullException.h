//
// Created by jaychen on 18-7-29.
//

#ifndef OS_HOMEWORK_CXX_SPACEFULLEXCEPTION_H
#define OS_HOMEWORK_CXX_SPACEFULLEXCEPTION_H

#endif //OS_HOMEWORK_CXX_SPACEFULLEXCEPTION_H

#include <exception>
using std::exception;

class spaceFullException :public exception
{
public:
    int exceptionCode;//0-》inode,1-》space
    spaceFullException(int exceptionCode);
    char* what();

};