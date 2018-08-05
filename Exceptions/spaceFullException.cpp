//
// Created by jaychen on 18-7-29.
//

#include "spaceFullException.h"

spaceFullException::spaceFullException(int exceptionCode) {
    this->exceptionCode=exceptionCode;
}

char *spaceFullException::what()
{
    if(exceptionCode==0)
    {
        return "No more space for new inode!\n";
    }
    else
    {
        return "No more space for new file!\n";
    }
}
