//
// Created by jaychen on 18-7-28.
//

#ifndef OS_HOMEWORK_CXX_INODEMANAGER_H
#define OS_HOMEWORK_CXX_INODEMANAGER_H

#include "Enum.h"

class InodeManager
{
public:
    static InodeManager* getInstance();
    void initialize();
    int findEmptyInode();
    int newINode(string filePermission, int size, int address,string owner);
    void setINode(int inode_number,int filePermission, int size, int address);
    void changeInodeBit(int inode_number,char* bit);
    string getAddress(string inode_number);
    fstream openInodeFile(string inode_number,open_mode mode);
private:
    InodeManager();
    static InodeManager* InodeManagerInstance;
};

#endif //OS_HOMEWORK_CXX_INODEMANAGER_H


