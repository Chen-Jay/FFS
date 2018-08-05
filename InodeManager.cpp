//
// Created by jaychen on 18-7-28.
//
#include <iostream>

using std::cin;
using std::cout;
using std::endl;

#include <fstream>

using std::fstream;

#include <string>

using std::string;

#include <sstream>

using std::stringstream;

#include "InodeManager.h"
#include "DirectoryManager.h"
#include "./Exceptions/spaceFullException.h"

InodeManager *InodeManager::InodeManagerInstance = nullptr;

/**
 * 单例实现
 * @return 返回单例对象的指针
 */
InodeManager *InodeManager::getInstance() {
    if (InodeManagerInstance == nullptr) {
        InodeManagerInstance = new InodeManager;
    }
    return InodeManagerInstance;
}

/**
 * 构造函数
 */
InodeManager::InodeManager() {

}

/**
 * 寻找最小的空闲inode，辅助节点分配
 * @return 最小空闲inode的编号
 */
int InodeManager::findEmptyInode() {
    fstream inode_bitmap;
    inode_bitmap.open("./inode/inode_bitmap", std::fstream::in);

    char inode_exist_ch;
    int inode_number;

    for (inode_number = 1; !inode_bitmap.eof(); inode_number++) {
        inode_bitmap.get(inode_exist_ch);
//        cout<<"the inode number is "<<inode_exist_ch<<endl;
        if (inode_exist_ch != '1') {
//            cout<<"the empty inode is "<<inode_number;
            return inode_number;
        }
    }
    throw (spaceFullException(0));
}

/**
 * 初始化工作
 */
void InodeManager::initialize() {
    /**
     * 创建inode专属文件夹，相当于磁盘分区，分出一块特别的区域用于inode的存储，并创建inode位图（一个用于指示inode的使用状态的位图），用于inode的管理
     */
    DirectoryManager::getInstance()->createFolder("inode");
    fstream inode_bitmap;
    inode_bitmap.open("./inode/inode_bitmap", std::fstream::app);
    inode_bitmap.close();

    /**
     * 如果发现inode位图为空，则对其进行初始化，使其最多拥有2097152个inode（即每一KB都分配一个inode，最多能存储2G的文件）
     */
    inode_bitmap.open("./inode/inode_bitmap", std::fstream::in);
    inode_bitmap.get();
    if (inode_bitmap.eof()) {
//        cout<<"the file is empty";
        inode_bitmap.close();
        inode_bitmap.open("./inode/inode_bitmap", std::fstream::app);
        for (int i = 0; i < 2097152; i++) {
            inode_bitmap.write("0", 1);
        }
        inode_bitmap.flush();
        inode_bitmap.close();
    }

    /**
     * 如果发现根目录的inode（1号inode）还没有被创建，则创建
     */
    fstream root_inode;
    root_inode.open("./inode/1", std::fstream::app);
    root_inode.close();
    root_inode.open("./inode/1", std::fstream::in);
    root_inode.get();
    if (root_inode.eof())
    {
        newINode("44", 1, 0,"");
    }
    root_inode.close();
}

/**
 * 用于改变对应inode的状态
 * @param inode_number inode号
 * @param bit 改变的状态（0/1）
 */
void InodeManager::changeInodeBit(int inode_number, char *bit) {
    fstream inode_bitmap;
    inode_bitmap.open("./inode/inode_bitmap", std::fstream::in | std::fstream::out);
    inode_bitmap.seekg(inode_number - 1);
    inode_bitmap.write(bit, 1);
    inode_bitmap.flush();
    inode_bitmap.close();
}

/**
 * 分配新的inode
 * @param filePermission 文件权限 如：777
 * @param size 文件大小（默认1KB）
 * @param address 物理地址
 */
int InodeManager::newINode(string filePermission, int size, int address,string owner)
{
    try {
        /**
         * 清空/创建空的inode文件
         */
        int inode_number = findEmptyInode();
        fstream inode;
        stringstream ss;
        ss << inode_number;
        string inode_file_name = "./inode/";
        inode_file_name += ss.str();
        inode.open(inode_file_name, std::fstream::out);

        /**
         * 写入物理地址
         */
        stringstream ss1;
        ss1 << address;
        inode.write(ss1.str().c_str(), ss1.str().length());
        inode.write("\n", 1);

        /**
         * 写入文件权限码
         */
        inode.write(filePermission.c_str(), filePermission.length());
        inode.write("\n", 1);

        /**
         * 写入文件大小
         */
        stringstream ss2;
        ss2 << size;
        inode.write(ss2.str().c_str(), ss2.str().length());
        inode.write("\n", 1);

        /**
         * 写入拥有者
         */
         inode.write(owner.c_str(),owner.length());
        inode.write("\n", 1);

        changeInodeBit(inode_number, "1");
        inode.flush();
        inode.close();
        return inode_number;
    }
    catch (spaceFullException ex) {
        cout << ex.what();
    }
}

/**
 * 修改对应编号的inode
 * @param inode_number 对应的inode号
 * @param filePermission 文件权限
 * @param size 文件大小
 * @param address 物理地址
 */
void InodeManager::setINode(int inode_number, int filePermission, int size, int address) {
    /**
     * 打开对应的inode文件
     */
    fstream inode;
    changeInodeBit(inode_number, "1");
    stringstream ss;
    ss << inode_number;
    string inode_file_name = "./inode/";
    inode_file_name += ss.str();
    inode.open(inode_file_name, std::fstream::out);

    /**
    * 写入物理地址
    */
    stringstream ss1;
    ss1 << address;
    inode.write(ss1.str().c_str(), ss1.str().length());
    inode.write("\n", 1);

    /**
     * 写入文件权限码
     */
    stringstream ss2;
    ss2 << filePermission;
    inode.write(ss2.str().c_str(), ss2.str().length());
    inode.write("\n", 1);

    /**
     * 写入文件大小
     */
    stringstream ss3;
    ss3 << size;
    inode.write(ss3.str().c_str(), ss3.str().length());

    inode.flush();
    inode.close();
}

/**
 * 根据inode号拿到该文件的物理地址
 * @param inode_number
 * @return 物理地址
 */
string InodeManager::getAddress(string inode_number) {
    string inode_file_name = "./inode/";
    inode_file_name += inode_number;
    fstream inode_dir;
    inode_dir.open(inode_file_name, std::fstream::in);
    string address;
    getline(inode_dir, address);
    inode_dir.close();
    return address;
}

fstream InodeManager::openInodeFile(string inode_number, open_mode mode) {
    fstream inode;
    string inode_file_name="./inode/";
    inode_file_name+=inode_number;

    if(mode==open_mode::in)
    {
        inode.open(inode_file_name,std::fstream::in);
    }
    else if(mode==open_mode::out)
    {
        inode.open(inode_file_name,std::fstream::out);
    }
    else if(mode==open_mode::in_out)
    {
        inode.open(inode_file_name,std::fstream::in|std::fstream::out);
    }
    else if(mode==open_mode::app)
    {
        inode.open(inode_file_name,std::fstream::app);
    }

    return inode;
}
