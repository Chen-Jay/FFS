//
// Created by jaychen on 18-7-18.
//

#ifndef OS_HOMEWORK_CXX_CONFIGURATION_H
#define OS_HOMEWORK_CXX_CONFIGURATION_H

#include <string>
using std::string;

#include <fstream>

using std::fstream;

#include "Enum.h"

class UserManager
{
public:
    /**
     * 内部使用接口
     */
    static UserManager* getInstance();
    void initialize();
    bool getLoginState();
    string getUserName();
    fstream openUserDataFile(open_mode);
    /**
     * 用户接口
     */
    void logIn();
    void signUp();
    void logOut();

private:
    UserManager();
    static UserManager* UserManagerInstance;
    string UserName;
    int IfLogin;
    bool validateUserName(string username);
    bool validatePassword(string password);
    bool checkPassword(string username,string password);
    bool checkUserName(string username);
    void setLoginState(bool logState);

};



#endif //OS_HOMEWORK_CXX_CONFIGURATION_H

