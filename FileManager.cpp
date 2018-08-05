//
// Created by jaychen on 18-7-28.
//

#include <fstream>
using std::fstream;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <sys/stat.h>
#include <cmath>

#include "FileManager.h"
#include "DirectoryManager.h"
#include "UserManager.h"
#include "InodeManager.h"
#include "./Exceptions/spaceFullException.h"
#include "./Exceptions/fileNotFoundException.h"
#include "./Exceptions/fileExistException.h"

FileManager* FileManager::FileManagerInstance=nullptr;

/**
 * 单例实现
 * @return 单例对象的指针
 */
FileManager *FileManager::getInstance() {
    if(FileManagerInstance== nullptr)
    {
        FileManagerInstance=new FileManager;
    }
    return FileManagerInstance;
}

FileManager::FileManager() {
    currentFile="";
}

/**
 * 创建一个文件
 * @param filename 文件名
 * @param permission 文件权限
 * @return 该文件的inode号
 */
string FileManager::createFile(string filename,string permission) {
    if(!ifNameExist(filename))
    {
        int address=findEmptySpace(1);
        int inode_number=InodeManager::getInstance()->newINode(permission,1,address,UserManager::getInstance()->getUserName());
        stringstream ss;
        ss<<inode_number;
        DirectoryManager::getInstance()->registerInDir(filename,ss.str());

        fstream file;
        string file_name="./file/file";
        stringstream ss2;
        ss2<<address;
        file_name+=ss2.str();
        file.open(file_name,std::fstream::app);
        file.close();
        stringstream ss3;
        ss3<<inode_number;
        return ss3.str();
    }
    else
    {
        throw(fileExistException());
    }

}

/**
 * 根据位图，找到空的所需空间的起始物理地址,更新位图
 * @param size 所需空间大小
 * @return 起始物理地址
 */
int FileManager::findEmptySpace(int size)
{
    /**
     * 打开位图，找到新的大小为size的空闲空间，返回该物理位置，并把对应的bit置为1
     */
    fstream space_bitmap;
    space_bitmap.open("./spaceBitmap/space_bitmap",std::fstream::in|std::fstream::out);
    int address=0;
    int pos=-1;
    int free_space=0;
    char buf;
    do
    {
        pos++;
        space_bitmap.get(buf);
        if(buf=='0')
        {
            free_space++;
            if(free_space>=size)
            {
                changeBit(address+1,address+free_space,"1");
                return address;
            }
        }
        else
        {
            if(free_space>size)
            {
                changeBit(address+1,address+free_space,"1");
                return address;
            }
            else
            {
                address=pos+1;
                free_space=0;
            }
        }
    }
    while(!space_bitmap.eof());
    throw (spaceFullException(1));
}

/**
 * 初始化工作
 */
void FileManager::initialize()
{
    /**
     * 分出特定的磁盘区域用于储存管理磁盘空间的位图
     */
    DirectoryManager::getInstance()->createFolder("spaceBitmap");
    fstream space_bitmap;
    space_bitmap.open("./spaceBitmap/space_bitmap",std::fstream::app);
    space_bitmap.close();
    space_bitmap.open("./spaceBitmap/space_bitmap",std::fstream::in);

    /**
     * 分出特定的磁盘区域用来存放文件
     */
    DirectoryManager::getInstance()->createFolder("file");

    /**
     * 如果该位图为空，则初始化位图（位图分配应该根据实际磁盘空间的大小来决定，但是这里我们模拟管理一个2G的磁盘，因此，每一KB对应一个二进制位，有2097152位）
     */
    space_bitmap.get();
    if(space_bitmap.eof())
    {
        space_bitmap.close();
        space_bitmap.open("./spaceBitmap/space_bitmap",std::fstream::out);
        for(int i=0;i<2097152;i++)
        {
            space_bitmap.write("0",1);
        }
        space_bitmap.flush();
        space_bitmap.close();
    }
}

/**
 * 将位图特定范围内的位换成指定的位
 * @param start_pos 开始位置
 * @param end_pos 结束位置
 * @param bit 想要换成的位
 */
void FileManager::changeBit(int start_pos, int end_pos, char *bit)
{
//    cout<<"start:"<<start_pos<<"\tend:"<<end_pos<<endl;
    fstream bitmap;
    bitmap.open("./spaceBitmap/space_bitmap",std::fstream::in|std::fstream::out);
    bitmap.seekp(start_pos-1);
    for(int i=0;i<=end_pos-start_pos;i++)
    {
        bitmap.write(bit,1);
    }
    bitmap.flush();
    bitmap.close();
}

/**
 * 得到当前目录下对应文件的地址
 * @param filename 文件名
 * @return 文件物理地址
 */
string FileManager::getFileAddress(string filename)
{
    /**
     * 打开当前目录
     */
    fstream dir;
    dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);

    string dir_content;
    string f_name;
    string inode;
    string address;
    do {
        getline(dir, dir_content);
        f_name = dir_content.substr(0, dir_content.find(" "));
        if (f_name==filename)
        {
            inode=dir_content.substr(dir_content.find(" ")+1,dir_content.length());
            address=InodeManager::getInstance()->getAddress(inode);
            return address;
        }
    }
    while(!dir.eof());
    throw (fileNotFoundException());
}



/**
 * 检查当前目录下有没有给定文件名的文件
 * @param filename 文件名
 * @return 是否有应该文件
 */
bool FileManager::ifNameExist(string filename)
{
    /**
     * 打开所在目录的目录文件
     */
     fstream dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);

    string dir_content;
    string filename_exist;
    int line=0;
    do
    {
        if(line<4)
        {
            getline(dir,dir_content);
            line++;
            continue;
        }
        getline(dir,dir_content);
        filename_exist=dir_content.substr(0,dir_content.find(" "));
        if(filename_exist==filename)
        {
            return true;
        }
    }
    while(!dir.eof());
    return false;

}

/**
 * 打开对应目录下的对应文件
 * @param filename
 * @param mode
 * @return
 */
fstream FileManager::openFile(string filename, open_mode mode)
{
    try
    {
        string address = FileManager::getInstance()->getFileAddress(filename);
        fstream file;
        string file_address ="./file/file";
        file_address+=address;
        if(mode==open_mode::out)
        {
            file.open(file_address,std::fstream::out);
        }
        else if (mode==open_mode::in)
        {
            file.open(file_address,std::fstream::in);
        }
        else if(mode==open_mode::in_out)
        {
            file.open(file_address,std::fstream::in|std::fstream::out);
        }
        else if(mode==open_mode::app)
        {
            file.open(file_address,std::fstream::app);
        }
        return file;
    }
    catch (fileNotFoundException ex)
    {
        cout<<ex.what()<<endl;
    }

}

/**
 * 创建用户的文件
 * @param filename 文件名
 * @param permission 文件权限码
 */
void FileManager::createUserFile()
{
    /**
     * 检查是否登录
     */
    if(!UserManager::getInstance()->getLoginState())
    {
        cout<<"Please log in first!"<<endl;
        return;
    }

    /**
     * 检查是否在本用户目录下
     */
    if(UserManager::getInstance()->getUserName()!=DirectoryManager::getInstance()->getCurrentDir())
    {
        if(DirectoryManager::getInstance()->getCurrentDir()=="/")
        {
            cout<<"You can't create file in root directory! If you want to create file, you should go to your own directory first."<<endl;
            return;
        }
        else
        {
            cout<<"You can't create file in other user's directory! If you want to create file, you should go to your own directory first."<<endl;
            return;
        }
    }

    /**
     * 输入文件名并且对文件名做检查
     */
    cout<<"Please enter the file name: ";
    string filename;
    getline(cin,filename);
    bool result=true;
    if(filename.length()==0)
    {
        cout<<"The filename can not be empty!"<<endl;
        return;
    }

    for (char i : filename) {
        if(i =='/'|| i ==' ')
        {
            result=false;
            break;
        }
    }
    if(!result)
    {
        cout<<"Please input a legal filename! (The filename cannot contain any space or '/')"<<endl;
        return;
    }

    /**
     * 输入文件权限码
     */
   string filePermissionCode;
   cout<<"Please enter you file permission code, for example \"77\",the first number is the permission code of yourself,the second number is the permission code of other users.If you input nothing, it will be set automatically to\"74\""<<endl;
   getline(cin,filePermissionCode);
   if(filePermissionCode.length()==0)
   {
       filePermissionCode="74";
   }
   if(filePermissionCode.length()!=2)
   {
       cout<<"Please input a legal file permission code,which consisting of 2 number,each of them should be 1-7."<<endl;
       return;
   }

   for(char i:filePermissionCode)
   {
       if(i>'7'||i<'1')
       {
            cout<<"Please input a legal file permission code,which consisting of 2 number,each of them should be 1-7."<<endl;
           return;
       }
   }

   try
   {
       createFile(filename,filePermissionCode);
   }
   catch (fileExistException ex)
   {
       cout<<ex.what()<<endl;
   }
}

/**
 * 将文件权限码转化成文字，辅助展示
 * @param file_permissionCode
 * @return
 */
string FileManager::filePermissionTrans(char file_permissionCode) {
    switch (file_permissionCode)
    {
        case '1':
        {
            return "X";
        }
        case '2':
        {
            return "W";
        }
        case '3':
        {
            return "WX";
        }
        case '4':
        {
            return "R";
        }
        case '5':
        {
            return "RX";
        }
        case '6':
        {
            return "RW";
        }
        case '7':
        {
            return "RWX";
        }
    }

}

/**
 * 用户打开文件
 */
void FileManager::open()
{
    /**
     * 只有登录用户才能进行相关操作
     */
    if(UserManager::getInstance()->getUserName()=="")
    {
        cout<<"Please login first!"<<endl;
        return;
    }

    /**
     * 同时只能打开一个文件
     */
    if(getCurrentOpenFile()!="")
    {
        cout<<"Please close the file you have opened first! (The file you have opened is "<<getCurrentOpenFile()<<")."<<endl;
        return;
    }

    /**
     * 文件名不能为空
     */
    cout<<"Please enter the file's name which you want to open: ";
    string filename;
    getline(cin,filename);
    if(filename.length()==0)
    {
        cout<<"The filename can not be empty!"<<endl;
        return;
    }

    /**
     * 检查该文件是否存在
     */
    fstream current_dir= DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
    int line=0;
    string dir_content;
    string filename_has_exist;
    bool exist=false;
    do
    {
        if(line<4)
        {
            getline(current_dir,dir_content);
            line++;
            continue;
        }
        getline(current_dir,dir_content);
        filename_has_exist=dir_content.substr(0,dir_content.find(" "));
        if(filename==filename_has_exist)
        {
            exist=true;
            break;
        }
    }
    while(!current_dir.eof());

    if(!exist)
    {
        cout<<"There isn't such a file in this directory!"<<endl;
        return;
    }

    setCurrentOpenFile(filename);
    cout<<"Open file succesfully!"<<endl;
}

/**
 * currentFile的get函数
 * @return
 */
string FileManager::getCurrentOpenFile() {
    return this->currentFile;
}

/**
 * currentFile的set函数
 * @param file
 */
void FileManager::setCurrentOpenFile(string file) {
    this->currentFile=file;
}

/**
 * 用户关闭打开的文件
 */
void FileManager::close() {
    if(getCurrentOpenFile()=="")
    {
        cout<<"You have not opened any file!"<<endl;
        return;
    }
    else
    {
        setCurrentOpenFile("");
        cout<<"Close file successfully!"<<endl;
    }
}

/**
 * 读取打开的文件
 */
void FileManager::read() {
    if(getCurrentOpenFile()=="")
    {
        cout<<"You have not opened any file!"<<endl;
        return;
    }

    if(ifPermit(open_mode::in))
    {
        system("clear");
        /**
         * 获取文件信息，直接输出
         */
        fstream file=openFile(getCurrentOpenFile(),open_mode::in);
        string file_content;
        do
        {
            getline(file,file_content);
            cout<<file_content<<endl;
        }
        while(!file.eof());

        cout<<"Press \"enter\" to exit......";
        string a;
        getline(cin,a);
        system("clear");
    }
    else
    {
        cout<<"You are not allowed to read this file."<<endl;
        return;
    }


}

/**
 * 判定用户是否拥有相关操作的权限
 * @param mode 操作
 * @return 是否拥有相应权限
 */
bool FileManager::ifPermit(open_mode mode)
{
    string user=UserManager::getInstance()->getUserName();
    string filename=FileManager::getInstance()->getCurrentOpenFile();

    /**
     * 读取当前目录，获得打开文件的indoe号
     */
    fstream dir= DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
    string dir_content;
    string file_name;
    string inode_number;
    int line=0;
    do {
        if (line < 4) {
            getline(dir, dir_content);
            line++;
            continue;
        }
        getline(dir, dir_content);
        file_name = dir_content.substr(0, dir_content.find(" "));
        if (file_name == filename) {
            inode_number = dir_content.substr(dir_content.find(" ") + 1, dir_content.length());
            break;
        }
    }
    while(!dir.eof());

    /**
     * 打开inode，获得对应的文件权限码以及拥有该文件的用户名
     */
    fstream inode= InodeManager::getInstance()->openInodeFile(inode_number,open_mode::in);

    string permission_code;
    string file_user;
    string inode_content;
    line=0;
    do
    {
        getline(inode,inode_content);
        if(line==0)
        {
            line++;
            continue;
        }
        else if(line==1)
        {
            line++;
            permission_code=inode_content;
        }
        else if(line==2)
        {
            line++;
            continue;
        }
        else if(line==3)
        {
            line++;
            file_user=inode_content;
        }
    }
    while(!inode.eof());

    /**
     * 根据当前用户与文件拥有者的关系，判定是否允许进行相关操作
     */
    if(file_user==user)
    {
        char p_code=permission_code[0];
        if(mode==open_mode::in)
        {
            return p_code == '4' || p_code == '5' || p_code == '6' || p_code == '7';
        }
        else if(mode==open_mode::app||mode==open_mode::out)
        {
            return p_code == '2' || p_code == '3' || p_code == '6' || p_code == '7';
        }
    }
    else
    {
        char p_code=permission_code[1];
        if(mode==open_mode::in)
        {
            return p_code == '4' || p_code == '5' || p_code == '6' || p_code == '7';
        }
        else if(mode==open_mode::app||mode==open_mode::out)
        {
            return p_code == '2' || p_code == '3' || p_code == '6' || p_code == '7';
        }
    }
}

/**
 * 用户删除文件
 */
void FileManager::deleteFile() {
    /**
     * 检查用户是否已经打开文件
     */
    if(getCurrentOpenFile()=="")
    {
        cout<<"You have not opened any file!"<<endl;
        return;
    }

    /**
     * 判定用户是否拥有修改文件的权限，如果拥有，执行删除文件的流程
     */
    if(ifPermit(open_mode::out)) {
        /**
         * 拿到文件的物理地址
         */
        string filename = "./file/file";
        filename += getFileAddress(getCurrentOpenFile());

        /**
         * 打开当前目录，拿到文件的inode号
         */
        fstream dir = DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
        string dir_content;
        string filename_has_exist;
        string inode_number;
        int line=0;
        do
        {
            if(line<4)
            {
                getline(dir,dir_content);
                line++;
                continue;
            }
            getline(dir,dir_content);
            filename_has_exist=dir_content.substr(0,dir_content.find(" "));
            if(filename_has_exist==getCurrentOpenFile())
            {
                inode_number=dir_content.substr(dir_content.find(" ")+1,dir_content.length());
                break;
            }
            line++;

        }
        while(!dir.eof());

        /**
         * 从inode中，获得占用磁盘的开始位置，文件大小。
         */
        fstream inode=InodeManager::getInstance()->openInodeFile(inode_number,open_mode::in);
        string start_pos;
        string size;
        string inode_content;
        stringstream ss;

        getline(inode,start_pos);
        int start_pos_int;
        ss<<start_pos;
        ss>>start_pos_int;

        getline(inode,inode_content);

        getline(inode,size);
        stringstream ss1;
        ss1<<size;
        int size_int;
        ss1>>size_int;
        size_int--;

        inode.close();

        stringstream ss2;
        ss2<<inode_number;
        int inode_number_int;
        ss2>>inode_number_int;

        //将对应的inode在位图中去掉
        InodeManager::getInstance()->changeInodeBit(inode_number_int,"0");


        //将文件所占有的空间在位图清零
        FileManager::getInstance()->changeBit(start_pos_int+1,start_pos_int+size_int+1,"0");
        //将文件删除
        remove(filename.c_str());
        //在目录中把对应的行清空
        dir.close();
        dir= DirectoryManager::getInstance()->openCurrentDir(open_mode::in_out);

        /**
         * 将原来目录里面的内容存放在一个<string>数组中
         */
        line=1;
        int file_line;
        vector<string> content;
        do
        {
            getline(dir,dir_content);
            if(dir_content.substr(0,dir_content.find(" "))==getCurrentOpenFile())
            {
                file_line=line;
            }
            line++;
            content.push_back(dir_content);
        }
        while(!dir.eof());
        dir.close();

        /**
         * 将原来目录里面的内容（除了被删除文件以外的信息）写会目录文件
         */
        dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::out);
        for(int i=1;i<=content.size();i++)
        {
            if(i==file_line)
            {
                continue;
            }
            dir.write(content[i-1].c_str(),content[i-1].length());
            dir.write("\n",1);
        }
        dir.close();
        close();
    }
    else
    {
        cout<<"You are not allowed to delete this file."<<endl;
        return;
    }
}

/**
 * 用户写文件
 */
void FileManager::write() {
    /**
     * 检查用户是否已经打开文件
     */
    if(getCurrentOpenFile()=="")
    {
        cout<<"You have not opened any file!"<<endl;
        return;
    }

    /**
     * 判定用户是否拥有修改文件的权限，如果拥有，执行改写文件的流程
     */
    if(ifPermit(open_mode::out))
    {
        string filename="./file/file";
        filename+=getFileAddress(getCurrentOpenFile());
        int size_before=getFileSize(const_cast<char *>(filename.c_str()));
        if(size_before==0)
        {
            size_before=1;
        }
        cout<<"Please enter the mode you that you want to write this file (\"append\" or \"rewrite\"),or you can \"exit\":";
        string mode;
        getline(cin,mode);
        if(mode=="append")//如果用户以添加内容的方式打开文件
        {

            system("clear");
            cout<<"Now, please enter the content you want to append. ";
            cout<<"Each line of the content will be appended to the file after an \"Enter\" was input.";
            cout<<" If you want to finish your editing, please input \"Ctrl+D\""<<endl<<endl;
            cout<<"Press \"Enter\" to start editing.";
            string line;
            getline(cin,line);
            system("clear");

            /**
             * 查看文件是否为空
             */
            bool ifEmpty=false;
            fstream file=openFile(getCurrentOpenFile(),open_mode::in);
            file.get();
            if(file.eof())
            {
                ifEmpty=true;
            }
            file.close();

            /**
             * 一边读入内容，一边append文件内容
             */
            file= FileManager::getInstance()->openFile(getCurrentOpenFile(),open_mode::app);
            int count=1;
            do
            {
                if(ifEmpty)
                {
                    count++;
                    try
                    {
                        getline(cin,line);
                    }
                    catch (std::ios::failure ex)
                    {
                        ;
                    }
                    if(!cin.eof())
                    {
                        file.write(line.c_str(),line.length());
                        file.flush();
                        ifEmpty=false;
                    }
                }
                else
                {
                    try
                    {
                        getline(cin,line);
                    }
                    catch (std::ios::failure ex)
                    {
                        ;
                    }

                    if(!cin.eof())
                    {
                        file.write("\n",1);
                        file.write(line.c_str(),line.length());
                        file.flush();
                    }
                }

            }
            while(!cin.eof());

            file.close();
            int size_after=getFileSize(const_cast<char *>(filename.c_str()));
            string inode_numbner;

            /**
             * 如果发现文件前后大小发生了改变（以KB为单位），则为文件找寻新的物理地址，并且更新inode
             */
            if(size_after!=size_before)
            {

                /**
                 * 打开当前目录，获得文件的inode号
                 */
                fstream dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
                string dir_content;
                do
                {
                    getline(dir,dir_content);
                    if(dir_content.substr(0,dir_content.find(" "))==getCurrentOpenFile())
                    {
                        inode_numbner=dir_content.substr(dir_content.find(" ")+1,dir_content.length());
                        break;
                    }

                }
                while(!dir.eof());
                dir.close();

                /**
                 * 将原来inode里面的内容记录下来
                 */
                fstream inode=InodeManager::getInstance()->openInodeFile(inode_numbner,open_mode::in);
                string address;
                string permissionCode;
                string size;
                string owner;

                getline(inode,address);
                getline(inode,permissionCode);
                getline(inode,size);
                getline(inode,owner);

                inode.close();

                /**
                 * 将文件原来占有的磁盘空间释放（将位图中对应的位设置为0）
                 */
                stringstream ss0;
                ss0<<address;
                int address_int;
                ss0>>address_int;
                stringstream ss1;
                ss1<<size;
                int size_int;
                ss1>>size_int;
                changeBit(address_int+1,address_int+size_int,"0");

                /**
                 * 为修改后的文件找新的位置，并且将位图相应的位设置为1
                 */
                int start_address=findEmptySpace(size_after);

                /**
                 * 更新inode内容
                 */
                 stringstream ss2;
                 ss2<<start_address;
                address=ss2.str();

                stringstream ss3;
                ss3<<size_after;
                size=ss3.str();

                inode=InodeManager::getInstance()->openInodeFile(inode_numbner,open_mode::out);
                inode.write(address.c_str(),address.length());
                inode.write("\n",1);
                inode.write(permissionCode.c_str(),permissionCode.length());
                inode.write("\n",1);
                inode.write(size.c_str(),size.length());
                inode.write("\n",1);
                inode.write(owner.c_str(),owner.length());
                inode.write("\n",1);

                /**
                 * 将原来地址空间中的内容删去，在新的物理地址空间中写入新内容
                 */
                fstream new_file;
                string new_filename="./file/file";
                new_filename+=address;
                rename(filename.c_str(),new_filename.c_str());
                cin.clear();
//                system("clear");
                return;
            }
            else
            {
                cin.clear();
                file.close();
//                system("clear");
                return;
            }
        }
        else if(mode=="rewrite") //如果用户以重写方式打开文件
        {
            system("clear");
            cout<<"Now, please enter the content you want to write. ";
            cout<<"Each line of the content will be writen to the file after an \"Enter\" was input.";
            cout<<" If you want to finish your editing, please input \"Ctrl+D\""<<endl<<endl;
            cout<<"Press \"Enter\" to start editing.";
            string line;
            getline(cin,line);
            system("clear");

            /**
             * 一边读取用户输入，一边重写文件
             */
            fstream file= FileManager::getInstance()->openFile(getCurrentOpenFile(),open_mode::out);
            int count=1;
            do
            {
                if(count==1)
                {
                    count++;
                    getline(cin,line);
                    if(!cin.eof())
                    {
                        file.write(line.c_str(),line.length());
                        file.flush();
                    }
                }
                else
                {
                    getline(cin,line);

                    if(!cin.eof())
                    {
                        file.write("\n",1);
                        file.write(line.c_str(),line.length());
                        file.flush();
                    }
                }

            }
            while(!cin.eof());
            file.close();


            int size_after=getFileSize(const_cast<char *>(filename.c_str()));
            string inode_numbner;

            /**
             * 如果发现重写前后文件大小不一样（KB为单位），则重新分配空间，改写inode中相关内容
             */
            if(size_after!=size_before)
            {

                /**
                 * 打开当前目录，获得文件的inode号
                 */
                fstream dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
                string dir_content;
                do
                {
                    getline(dir,dir_content);
                    if(dir_content.substr(0,dir_content.find(" "))==getCurrentOpenFile())
                    {
                        inode_numbner=dir_content.substr(dir_content.find(" ")+1,dir_content.length());
                        break;
                    }

                }
                while(!dir.eof());
                dir.close();

                /**
                 * 将原来inode里面的内容记录下来
                 */
                fstream inode=InodeManager::getInstance()->openInodeFile(inode_numbner,open_mode::in);
                string address;
                string permissionCode;
                string size;
                string owner;

                getline(inode,address);
                getline(inode,permissionCode);
                getline(inode,size);
                getline(inode,owner);

                inode.close();

                /**
                 * 将文件原来占有的磁盘空间释放（将位图中对应的位设置为0）
                 */
                stringstream ss0;
                ss0<<address;
                int address_int;
                ss0>>address_int;
                stringstream ss1;
                ss1<<size;
                int size_int;
                ss1>>size_int;
                changeBit(address_int+1,address_int+size_int,"0");

                /**
                 * 为修改后的文件找新的位置，并且将位图相应的位设置为1
                 */
                int start_address=findEmptySpace(size_after);

                /**
                 * 更新inode内容
                 */
                stringstream ss2;
                ss2<<start_address;
                address=ss2.str();

                stringstream ss3;
                ss3<<size_after;
                size=ss3.str();

                inode=InodeManager::getInstance()->openInodeFile(inode_numbner,open_mode::out);
                inode.write(address.c_str(),address.length());
                inode.write("\n",1);
                inode.write(permissionCode.c_str(),permissionCode.length());
                inode.write("\n",1);
                inode.write(size.c_str(),size.length());
                inode.write("\n",1);
                inode.write(owner.c_str(),owner.length());
                inode.write("\n",1);

                /**
                 * 将原来地址空间中的内容删去，在新的物理地址空间中写入新内容
                 */
                fstream new_file;
                string new_filename="./file/file";
                new_filename+=address;
                rename(filename.c_str(),new_filename.c_str());
                cin.clear();
//                system("clear");
                return;
            }
            else
            {
                cin.clear();
                file.close();
//                system("clear");
                return;
            }
        }
        else if(mode=="exit")
        {
            return;
        }
        else
        {
            cout<<"Please enter a legal mode!(append or write)"<<endl;
            return;
        }
    }
    else
    {
        cout<<"You are not allowed to delete this file."<<endl;
        return;
    }
}

/**
 * 返回文件的大小（单位->KB）
 * @param filename 文件名
 * @return
 */
int FileManager::getFileSize(char *filename) {
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;
    return ceil(size*1.0/1024.0);
}

/**
 * 修改文件的权限保护码
 */
void FileManager::changeMode()
{
    if(getCurrentOpenFile()=="")
    {
        cout<<"You haven't open any file!"<<endl;
        return;
    }

    fstream dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
    string dir_content;
    string inode_num;
    do
    {
        getline(dir,dir_content);
        if(dir_content.substr(0,dir_content.find(" "))==getCurrentOpenFile())
        {
            inode_num=dir_content.substr(dir_content.find(" ")+1,dir_content.length());
            break;
        }
    }
    while(!dir.eof());
    dir.close();

    fstream inode=InodeManager::getInstance()->openInodeFile(inode_num,open_mode::in);

    string address;
    getline(inode,address);

    string filePermissionCode;
    getline(inode,filePermissionCode);

    string size;
    getline(inode,size);

    string user;
    getline(inode,user);

    inode.close();

    if(user!=UserManager::getInstance()->getUserName())
    {
        cout<<"You are not allowed to change mode."<<endl;
        return;
    }

    cout<<"Please enter the file permission code:";
    getline(cin,filePermissionCode);
    if(filePermissionCode.length()!=2)
    {
        cout<<"Please input a legal file permission code,which consisting of 2 number,each of them should be 1-7."<<endl;
        return;
    }

    for(char i:filePermissionCode)
    {
        if(i>'7'||i<'1')
        {
            cout<<"Please input a legal file permission code,which consisting of 2 number,each of them should be 1-7."<<endl;
            return;
        }
    }

    inode=InodeManager::getInstance()->openInodeFile(inode_num,open_mode::out);
    inode.write(address.c_str(),address.length());
    inode.write("\n",1);
    inode.write(filePermissionCode.c_str(),filePermissionCode.length());
    inode.write("\n",1);
    inode.write(size.c_str(),size.length());
    inode.write("\n",1);
    inode.write(user.c_str(),user.length());
    inode.write("\n",1);
    inode.flush();
    inode.close();
    cout<<"Change mode successfully!"<<endl;
}
