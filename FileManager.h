//
// Created by jaychen on 18-7-28.
//

#ifndef OS_HOMEWORK_CXX_FILEMANAGER_H
#define OS_HOMEWORK_CXX_FILEMANAGER_H


#include <string>
using std::string;

#include <fstream>
using std::fstream;

#include "Enum.h"

class FileManager
{
public:
    /**
     * 内部使用接口
     */
    static FileManager* getInstance();
    string createFile(string filename,string permission);
    int findEmptySpace(int size);
    void changeBit(int start_pos, int end_pos, char *bit);
    void initialize();
    bool ifNameExist(string filename);
    string getFileAddress(string filename);
    fstream openFile(string filename,open_mode mode);
    string filePermissionTrans(char file_permissionCode);
    string getCurrentOpenFile();
    void setCurrentOpenFile(string file);
    bool ifPermit(open_mode mode);
    int getFileSize(char* filename);

    /**
     * (外部)用户接口
     */
     void createUserFile();
     void open();
     void close();
     void read();
     void deleteFile();
     void write();
     void changeMode();
private:
    FileManager();
    static FileManager* FileManagerInstance;
    string currentFile;
};



#endif //OS_HOMEWORK_CXX_FILEMANAGER_H
