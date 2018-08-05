//
// Created by jaychen on 18-7-27.
//

#ifndef OS_HOMEWORK_CXX_DIRECTORYMANAGER_H
#define OS_HOMEWORK_CXX_DIRECTORYMANAGER_H

#include <string>
using std::string;

#include <fstream>
using std::fstream;

#include "Enum.h"

class DirectoryManager
{
public:
    /**
     * 内部接口
     */
    static DirectoryManager* getInstance();
    void initialize();
    void createFolder(string dirname);
    void createUserDirectory(string username);
    fstream openCurrentDir(open_mode mode);

    bool isRoot();
    void setCurrentDir(string currentDir);
    string getCurrentDir();
    string getCurrentDirAddress();
    void registerInDir(string fileName,string inode_number);
    void showInodeContent(string filename,string inode_number);

    /**
     * 外部接口
     */
    void cd(string directory);
    void dir();
private:
    DirectoryManager();
    static DirectoryManager* DirectoryManagerInstance;
    string currentDir;
};


#endif //OS_HOMEWORK_CXX_DIRECTORYMANAGER_H

