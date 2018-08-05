//
// Created by jaychen on 18-7-30.
//

#include "fileNotFoundException.h"


char *fileNotFoundException::what()
{
    if(exception_code==-1||exception_code==0)
    {
        return "There is no such a file in the directory!";
    }
    else if(exception_code==1)
    {
        return "There is no such a directory!";
    }
}

fileNotFoundException::fileNotFoundException() {
    exception_code=-1;
}

fileNotFoundException::fileNotFoundException(int exception_code) {
    this->exception_code=exception_code;
}
