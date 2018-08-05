//
// Created by jaychen on 18-7-30.
//

#include "fileExistException.h"

char *fileExistException::what() {
    return "There is such a file in the directory!";
}
