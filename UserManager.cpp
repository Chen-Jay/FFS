//
// Created by jaychen on 18-7-18.
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

#include "UserManager.h"
#include "FileManager.h"
#include "DirectoryManager.h"
#include "InodeManager.h"
#include "MD5.h"
#include "./Exceptions/spaceFullException.h"
#include "./Exceptions/fileExistException.h"
#include "./Exceptions/fileNotFoundException.h"

UserManager *UserManager::UserManagerInstance = nullptr;

UserManager::UserManager() {
    IfLogin = false;
    UserName = "";
}

/**
 * 单例实现
 * @return UserManager对象指针
 */
UserManager *UserManager::getInstance() {
    if (UserManagerInstance == nullptr) {
        UserManagerInstance = new UserManager;
    }
    return UserManagerInstance;
}

/**
 * 私有函数，用于辅助登录状态管理
 * @param logState 改变后的登录状态
 */
void UserManager::setLoginState(bool logState) {
    IfLogin = logState;
}

/**
 * 公有函数，用于获取登录状态
 * @return
 */
bool UserManager::getLoginState() {
    return IfLogin;
}

/**
 * 公有函数，用于获取用户名
 * @return
 */
string UserManager::getUserName() {
    return UserName;
}

/**
 * 共有函数，开放的接口
 * 用户登录
 */
void UserManager::logIn() {
    if (getLoginState() == false)
    {
        /**
         * 只允许在根目录下登录
        */
        if (DirectoryManager::getInstance()->getCurrentDir() != "/") {
            cout << "If you want to login,please go to the root directory" << endl;
            return;
        }

        cout << "Please enter your username: ";
        string username;
        getline(cin, username);
        if(!checkUserName(username))
        {
            return;
        }

        string password;
        cout << "Please enter your password: ";

        getline(cin, password);
        if(checkPassword(username, password) == false)
        {
            return;
        }

        UserManager::getInstance()->UserName = username;
        UserManager::getInstance()->setLoginState(true);
        system("clear");
    }
    else
    {
        cout << "you have log in!" << endl;
    }
}

/**
 * 共有函数，开放的接口
 * 用户注册
 */
void UserManager::signUp()
{
    /**
     * 只允许在根目录下注册
     */
    if (DirectoryManager::getInstance()->getCurrentDir() != "/") {
        cout << "If you want to sign up a new account,please go to the root directory first." << endl;
        return;
    }
    /**
     * 输入用户名并做相关合法性检验
     */
    cout << "Please enter your username,it can only contain the numbers,characters and '_'." << endl;
    string username;
    bool isValidate;
    getline(cin, username);
    isValidate = validateUserName(username);
    if (!isValidate)
    {
        cout << "Please enter a legal user name!" << endl;
        return;
    }


    /**
     * 输入密码并做相关合法性检验
     */
    cout << "Please enter your password." << endl;
    string password;
    do {
        getline(cin, password);
    } while (!validatePassword(password));
    MD5 MD5_coder;
    password = MD5_coder.getMD5(password);

    fstream UserFile=openUserDataFile(open_mode::app);
    /**
     * 用户信息写入对应文件
     */
    UserFile.write(username.c_str(), username.length());
    UserFile.write(" ", 1);
    UserFile.write(password.c_str(), password.length());
    UserFile.write("\n", 1);
    UserFile.flush();
    UserFile.close();

    /**
     * 用户目录创建以及inode的分配，根目录的更新
     */
    fstream root_dir;
    try {
        DirectoryManager::getInstance()->createUserDirectory(username);
        system("clear");
        cout << "Sign up successfully!" << endl;
    }
    catch (spaceFullException ex) {
        cout << ex.what();
    }

}

/**
 * 公有函数，用户登出
 */
void UserManager::logOut() {
    if (!getLoginState()) {
        cout << "you haven't log in!" << endl;
    } else {
        setLoginState(false);
        UserManager::getInstance()->UserName = "";
        FileManager::getInstance()->close();
        cout << "Log out!" << endl;
    }
}

/**
 * 私有函数，用户辅助注册
 * 检验用户输入用户名的合法性
 * @param username 用户输入的用户名
 * @return 合法->true 不合法->false
 */
bool UserManager::validateUserName(string username) {
    bool result_2 = true;
    bool result_1 = true;

    //判定用户名中是否为空
    if(username.length()==0)
    {
        cout<<"Username can not be empty!"<<endl;
        return false;
    }

    //判定用户名中是否含有非法字符
    for (char i : username) {
        if ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z') || i == '_') {
            continue;
        } else {
            result_1 = false;
//          cout<<"the ilegal character is "<<username[i]<<"!"<<endl;
            break;
        }
    }

    //判定该用户名是否已经存在
    fstream UserFile;
    UserFile=openUserDataFile(open_mode::in);
    string username_password;
    string username_has_exist;
    while (!UserFile.eof()) {
        getline(UserFile, username_password);
        if (username_password.length() == 0) {
            break;
        }
        username_has_exist = string(username_password.substr(0, username_password.find(' ', 0)));
        if (username_has_exist == username) {
            cout << "The username has exist!" << endl;
            result_2 = false;
        }
    }
    UserFile.close();

    return result_1 && result_2;
}

/**
 * 私有函数，用于辅助注册
 * 检验用户输入密码的合法性
 * @param password 用户输入的密码
 * @return 合法->true 不合法->false
 */
bool UserManager::validatePassword(string password) {
    if (password.length() == 0) {
        cout << "The password can not be empty!" << endl;
        return false;
    }

    for (char i : password) {
        if ((i >= '0' && i <= '9') || (i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z')) { ;
        } else {
            cout << "Please enter legal password!" << endl;
            return false;
        }
    }

    return true;
}

/**
 * 私有函数，用于辅助登录
 * 检验对应的用户名下密码是否正确
 * @param username 用户输入的用户名
 * @param password 用户输入的密码
 * @return 两者匹配->true 两者不匹配->false
 */
bool UserManager::checkPassword(string username, string password) {
    fstream UserFile;
    MD5 MD5_coder;
    string username_password;
    string username_has_exist;
    string password_has_exist;
    UserFile=openUserDataFile(open_mode::in);
    while (!UserFile.eof()) {
        getline(UserFile, username_password);
        if (username_password.length() == 0) {
            break;
        }
        username_has_exist = string(username_password.substr(0, username_password.find(' ', 0)));
        password_has_exist = string(
                username_password.substr(username_password.find(' ', 0) + 1, username_password.length() - 1));
//        cout<<"username in file is: "<<username_has_exist<<endl;
//        cout<<"password in file is: "<<password_has_exist<<endl;
        if (username == username_has_exist) {
            if (string(MD5_coder.getMD5(password)) == password_has_exist) {
                cout << "Log in successfully." << endl;
                return true;
            } else {
                cout << "Password wrong!" << endl;
                return false;
            }
        }
    }
}

/**
 * 私有函数，辅助登录
 * 检验登录时输入的用户名是否存在
 * @param username 用户输入的用户名
 * @return 存在->true,不存在->false
 */
bool UserManager::checkUserName(string username) {
    fstream UserFile;
    string username_password;
    string username_has_exist;
    UserFile=openUserDataFile(open_mode::in);

    //读取文件的每一行来查看用户名
    while (!UserFile.eof()) {
        getline(UserFile, username_password);
        if (username_password.length() == 0) {
            break;
        }
        username_has_exist = string(username_password.substr(0, username_password.find(' ', 0)));
//        cout<<"username in file is: "<<username_has_exist<<endl;
        if (username == username_has_exist) {
            UserFile.close();
            return true;
        }
    }
    UserFile.close();
    cout << "Username doesn't exist!" << endl;
    return false;
}

/**
 * 初始化，创建用于储存用户信息的文件
 */
void UserManager::initialize() {
    fstream dir=DirectoryManager::getInstance()->openCurrentDir(open_mode::in);
    bool result= false;
    string dir_content;
    string filename_exist;
    do
    {
        getline(dir,dir_content);
        filename_exist=dir_content.substr(0,dir_content.find(" "));
        if(filename_exist=="User.dat")
        {
            result= true;
            break;
        }
    }
    while(!dir.eof());

    if(!result)
    {
        int address=FileManager::getInstance()->findEmptySpace(1);
        int inode_number=InodeManager::getInstance()->newINode("44",1,address,"");
        stringstream ss;
        ss<<inode_number;
        DirectoryManager::getInstance()->registerInDir("User.dat",ss.str());

        fstream file;
        string file_name="./file/file";
        stringstream ss2;
        ss2<<address;
        file_name+=ss2.str();
        file.open(file_name,std::fstream::app);
        file.close();
    }


}

/**
 * 打开用于储存用户信息的文件（注意必须在当前目录为根目录的情况下才能使用）
 * @return Use.dat的fstream
 */
fstream UserManager::openUserDataFile(open_mode mode)
{
    try
    {
        string address = FileManager::getInstance()->getFileAddress("User.dat");
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

