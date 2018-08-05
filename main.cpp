#include <iostream>
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <fstream>
#include <cmath>

using std::fstream;

#include "UserManager.h"
#include "DirectoryManager.h"
#include "InodeManager.h"
#include "FileManager.h"
#include "./Exceptions/fileNotFoundException.h"

void showHelp();


int main()
{
    FileManager::getInstance()->initialize();
    InodeManager::getInstance()->initialize();
    DirectoryManager::getInstance()->initialize();
    UserManager::getInstance()->initialize();


    string command;
    system("clear");
    showHelp();
    cout<<endl;
    do
    {
        cout<<UserManager::getInstance()->getUserName()<<":"<<DirectoryManager::getInstance()->getCurrentDir()<<"$ ";
        getline(cin,command);
        if(command.substr(0,command.find(" "))=="cd")
        {
            try
            {
                DirectoryManager::getInstance()->cd(command.substr(command.find(" ")+1,command.length()));
            }
            catch (fileNotFoundException ex)
            {
                cout<<ex.what()<<endl;
                continue;
            }
        }
        else if(command=="dir"||command=="ls")
        {
            DirectoryManager::getInstance()->dir();
        }
        else if (command=="open")
        {
            FileManager::getInstance()->open();
        }
        else if(command=="close")
        {
            FileManager::getInstance()->close();
        }
        else if(command=="read")
        {
            FileManager::getInstance()->read();
        }
        else if(command=="write")
        {
            FileManager::getInstance()->write();
        }
        else if(command=="chmod")
        {
            FileManager::getInstance()->changeMode();
        }
        else if(command=="delete")
        {
            FileManager::getInstance()->deleteFile();
        }
        else if(command == "logout"||command=="log out")
        {
            UserManager::getInstance()->logOut();
        }
        else if(command== "login"||command=="log in")
        {
            UserManager::getInstance()->logIn();
        }
        else if(command=="signup"||command=="sign up")
        {
            UserManager::getInstance()->signUp();
        }
        else if(command=="create")
        {
            FileManager::getInstance()->createUserFile();
        }
        else if(command=="help")
        {
            showHelp();
        }
        else if(command=="exit")
        {
            return 1;
        }
        else
        {
            cout<<"Unknown command."<<endl;
        }
        cin.clear();
    }
    while(1);

}

void showHelp()
{
    cout<<"Command list:"<<endl;
    cout<<"help ------ check the command list"<<endl;
    cout<<"cd xx(xx are both directory) ------- go to the directory xx (you can use \".\" \"..\" and \"/\")"<<endl;
    cout<<"signup ------ sign up an account"<<endl;
    cout<<"login ------ start use these file system"<<endl;
    cout<<"logout ------- log out the account"<<endl;
    cout<<"create ------ create a file in current directory"<<endl;
    cout<<"open ------ open a file in current directory"<<endl;
    cout<<"chmod ------ change the file permission code of the file opened"<<endl;
    cout<<"write ------ write the file opened"<<endl;
    cout<<"read ------ read the file opened"<<endl;
    cout<<"delete ------ delete the file opened"<<endl;
    cout<<"close ------ close the file opened"<<endl;
    cout<<"dir ------ show the file/directory list in current directory"<<endl;
    cout<<"ls ------ show the file/directory list in current directory"<<endl;
    cout<<"exit ------ exit the system"<<endl;
}