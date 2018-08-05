//
// Created by jaychen on 18-7-27.
//

#include "DirectoryManager.h"
#include "InodeManager.h"
#include "FileManager.h"
#include "./Exceptions/fileNotFoundException.h"

#include <iostream>

using std::cin;
using std::cout;
using std::endl;

//用于目录的创建
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>

using std::fstream;

#include <sstream>

using std::stringstream;

#include <iomanip>

using std::setw;

DirectoryManager *DirectoryManager::DirectoryManagerInstance = nullptr;

DirectoryManager::DirectoryManager() {
    currentDir = "/";
}

/**
 * 单例实现
 * @return 返回单例对象的指针
 */
DirectoryManager *DirectoryManager::getInstance() {
    if (DirectoryManagerInstance == nullptr) {
        DirectoryManagerInstance = new DirectoryManager;
    }
    return DirectoryManagerInstance;
}

/**
 * 初始化工作
 */
void DirectoryManager::initialize() {
    /**
     * 如果根目录文件还没有创建，就创建根目录文件
     */
    fstream root_inode;
    root_inode.open("./inode/1", std::fstream::in);
    string root_address;
    getline(root_inode, root_address);
    root_inode.close();

    fstream root_dir;
    string root_dir_file_name = "./file/file";
    root_dir_file_name += root_address;
    root_dir.open(root_dir_file_name, std::fstream::app);
    root_dir.close();

    root_dir.open(root_dir_file_name, std::fstream::in);
    root_dir.get();
    if (root_dir.eof()) {
        FileManager::getInstance()->findEmptySpace(1);
        root_dir.close();
        root_dir.open(root_dir_file_name, std::fstream::app);
//        root_dir.write(". 1\n",4);
//        root_dir.write(".. 1\n",5);
        root_dir << ". 1\n" << ".. 1\n" << "/ 1\n";
    }
    root_dir.close();
}

/**
 * 创建文件夹
 * @param dirname
 */
void DirectoryManager::createFolder(string dirname) {
    mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
}

/**
 * 获取当前目录
 * @return 返回当前目录的名字
 */
string DirectoryManager::getCurrentDir() {
    return currentDir;
}

/**
 * 判断当前目录是否为根目录
 * @return 是->true  否->false
 */
bool DirectoryManager::isRoot() {
    if (currentDir == "/") {
        return true;
    } else {
        return false;
    }
}

/**
 * 在目录中完成对文件的登记（文件名 inode号）
 * @param fileName 文件名
 * @param inode_number inode号
 * @return 是否完成
 */
void DirectoryManager::registerInDir(string fileName, string inode_number) {

    string dir_file_name = "./file/file";
    dir_file_name += getCurrentDirAddress();
    fstream directory;
    directory.open(dir_file_name, std::fstream::in | std::fstream::out);
    string dir_content;
    int filename_end;
    string filename;
    int line=0;
    do {
        if(line<4)
        {
            getline(directory, dir_content);
            line++;
            continue;
        }
        getline(directory, dir_content);
        filename_end = dir_content.find(" ");
        filename = dir_content.substr(0, filename_end);
        if (filename == fileName) {
            cout << "the file has been registered!" << endl;
            directory.close();
            return;
        }
    } while (!directory.eof());
    directory.close();

    directory.open(dir_file_name, std::fstream::app);
    directory.write(fileName.c_str(), fileName.length());
    directory.write(" ", 1);
    directory.write(inode_number.c_str(), inode_number.length());
    directory.write("\n", 1);
    directory.flush();
    directory.close();
}

/**
 * 获取到当前目录的目录文件的地址
 * @return 文件的物理地址
 */
string DirectoryManager::getCurrentDirAddress() {
    string current_dir = getCurrentDir();

    /**
     * 如果当前目录为根目录，则返回从inode中获得的根目录的物理地址
     */
    string root_address;
    root_address = InodeManager::getInstance()->getAddress("1");
    if (current_dir == "/") {
        return root_address;
    }

    /**
     * 如果当前目录不是根目录，则进一步查找
     * 读取根目录文件，根据目录名获得当前目录的inode号
     */
    fstream root_dir;
    string root_dir_address = "./file/file";
    root_dir_address += root_address;
    root_dir.open(root_dir_address, std::fstream::in);
    string dir_content;
    string filename;
    string current_dir_inode_number;
    do {
        getline(root_dir, dir_content);
        int end = dir_content.find(" ");
        filename = dir_content.substr(0, end);
//        cout<<filename<<endl;
        if (filename == current_dir) {
            //获得当前目录的inode号
            current_dir_inode_number = dir_content.substr(end + 1, dir_content.length());
            break;
        }
    } while (!root_dir.eof());
    root_dir.close();

    /**
     * 根据当前目录的inode号，查找对应的inode文件，拿到当前目录的物理地址
     */
    fstream current_dir_inode;
    string current_dir_inode_file_name = "./inode/";
    current_dir_inode_file_name += current_dir_inode_number;
    current_dir_inode.open(current_dir_inode_file_name, std::fstream::in);
    string cur_address;
    getline(current_dir_inode, cur_address);
    current_dir_inode.close();
    return cur_address;
}

/**
 * 创建用户目录的目录文件(只在登录时会使用，默认在根目录)
 * @param currentDirInode 用户目录的inode
 */
void DirectoryManager::createUserDirectory(string username) {
    string inode = FileManager::getInstance()->createFile(username, "44");
    fstream new_dir = FileManager::getInstance()->openFile(username, open_mode::app);

    /**
     * 写入当前目录的inode号
     */
    new_dir.write(". ", 2);
    new_dir.write(inode.c_str(), inode.length());
    new_dir.write("\n", 1);

    new_dir.write(username.c_str(), username.length());
    new_dir.write(" ", 1);
    new_dir.write(inode.c_str(), inode.length());
    new_dir.write("\n", 1);

    /**
     * 写入上级目录的inode号（对于本系统来说就是根目录固定的inode号1）
     */
    new_dir.write(".. 1\n", 5);
    new_dir.write("/ 1\n", 4);

    new_dir.close();
}

/**
 * 目录跳转
 * @param directory
 */
void DirectoryManager::cd(string directory) {
    if (directory.length() == 0) {
        return;
    }

    if (directory == "/") {
        setCurrentDir("/");
        return;
    }

    fstream current_dir = openCurrentDir(open_mode::in);

    string dir_content;
    string directory_buf;
    directory_buf = directory.substr(0, directory.find("/"));
    do {
        getline(current_dir, dir_content);
        if (dir_content.substr(0, dir_content.find(" ")) == directory_buf) {
            if (directory_buf == ".")
            {
                ;
            }
            else if (directory_buf == "..")
            {
                setCurrentDir("/");
            }
            else if(directory_buf=="")
            {
                cout<<"Please input legal directory!"<<endl;
                return;
            }
            else
            {
                setCurrentDir(directory_buf);
            }
//                cout<<"current directory has changed to "<<directory_buf<<endl;
//                cout<<"the next diretory is"<<directory.substr(directory.find("/")+1,directory.length())<<endl;
//                string next_dir=;
            if (directory.find("/") == string::npos) {
                return;
            } else {
                cd(directory.substr(directory.find("/") + 1, directory.length()));
            }

            return;
        }
    } while (!current_dir.eof());
    throw (fileNotFoundException(1));
}

/**
 * 打开当前的目录文件
 * @param mode 打开模式
 * @return 已经绑定文件的fstream
 */
fstream DirectoryManager::openCurrentDir(open_mode mode) {
    string address = getCurrentDirAddress();
    fstream cur_dir;
    string dir_file_name = "./file/file";
    dir_file_name += address;
    if (mode == open_mode::app) {
        cur_dir.open(dir_file_name, std::fstream::app);
    } else if (mode == open_mode::in) {
        cur_dir.open(dir_file_name, std::fstream::in);
    } else if (mode == open_mode::in_out) {
        cur_dir.open(dir_file_name, std::fstream::in | std::fstream::out);
    } else if (mode == open_mode::out) {
        cur_dir.open(dir_file_name, std::fstream::out);
    }

    return cur_dir;

}

/**
 * 改变当前目录
 * @param currentDir
 */
void DirectoryManager::setCurrentDir(string currentDir) {
    DirectoryManager::currentDir = currentDir;
}

/**
 * 显示当前目录情况
 */
void DirectoryManager::dir() {

//    cout << "file permission code\t"<<"user,size\t"<<"physical address\t"<<"filename"<< endl;
    fstream dir = openCurrentDir(open_mode::in);
    string dir_content;
    string filename;
    string inode_number;

    int count = 0;
    do {
        //过滤掉目录相关信息
        if (count < 4) {
            getline(dir, dir_content);
            count++;
            continue;
        }

        getline(dir, dir_content);
        if(dir_content.length()==0)
        {
            continue;
        }
        filename = dir_content.substr(0, dir_content.find(" "));
        inode_number = dir_content.substr(dir_content.find(" ") + 1, dir_content.length());
        showInodeContent(filename, inode_number);
    } while (!dir.eof());
}

void DirectoryManager::showInodeContent(string filename, string inode_number) {
    fstream inode = InodeManager::getInstance()->openInodeFile(inode_number, open_mode::in);
    string filePermissionCode;
    string username;
    string size;
    string address;

    int line = 1;
    string inode_content;
    do {
        getline(inode, inode_content);
        switch (line)
        {
            case 1: {
                address = inode_content;
                line++;
                break;
            }
            case 2: {
                filePermissionCode = inode_content;
                line++;
                break;
            }
            case 3: {
                size = inode_content;
                line++;
                break;
            }
            case 4: {
                username = inode_content;
                line++;
                break;
            }
        }
    } while (!inode.eof());

    int k = 0;
    string filePermissionShow;
    for (char i:filePermissionCode) {
        filePermissionShow+=FileManager::getInstance()->filePermissionTrans(i);
        if (k == 0) {
            filePermissionShow+="-";
            k++;
        }
    }
    cout <<setw(8)<<filePermissionShow<< "\t";

    cout << username << "\t";
    cout << size<<"KB      "<< "\t";
    cout << "address"<<address << "\t";
    cout << filename << "\t";
    cout<<endl;
}


