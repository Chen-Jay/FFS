//
// Source code from https://baike.baidu.com/item/MD5/212708
//

#include<iostream>
#include<string>
#include "MD5.h"
using namespace std;
#define shift(x, n) (((x) << (n)) | ((x) >> (32-(n))))//右移的时候，高位一定要补零，而不是补充符号位
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476


void MD5::mainLoop(unsigned int M[])
{
    unsigned int f,g;
    unsigned int a=atemp;
    unsigned int b=btemp;
    unsigned int c=ctemp;
    unsigned int d=dtemp;
    for (unsigned int i = 0; i < 64; i++)
    {
        if(i<16){
            f=F(b,c,d);
            g=i;
        }else if (i<32)
        {
            f=G(b,c,d);
            g=(5*i+1)%16;
        }else if(i<48){
            f=H(b,c,d);
            g=(3*i+5)%16;
        }else{
            f=I(b,c,d);
            g=(7*i)%16;
        }
        unsigned int tmp=d;
        d=c;
        c=b;
        b=b+shift((a+f+k[i]+M[g]),s[i]);
        a=tmp;
    }
    atemp=a+atemp;
    btemp=b+btemp;
    ctemp=c+ctemp;
    dtemp=d+dtemp;
}

/*
*填充函数
*处理后应满足bits≡448(mod512),字节就是bytes≡56（mode64)
*填充方式为先加一个1,其它位补零
*最后加上64位的原来长度
*/
unsigned int* MD5::add(string str)
{
    unsigned int num=((str.length()+8)/64)+1;//以512位,64个字节为一组
    unsigned int *strByte=new unsigned int[num*16];    //64/4=16,所以有16个整数
    strlength=num*16;
    for (unsigned int i = 0; i < num*16; i++)
        strByte[i]=0;
    for (unsigned int i=0; i <str.length(); i++)
    {
        strByte[i>>2]|=(str[i])<<((i%4)*8);//一个整数存储四个字节，i>>2表示i/4 一个unsigned int对应4个字节，保存4个字符信息
    }
    strByte[str.length()>>2]|=0x80<<(((str.length()%4))*8);//尾部添加1 一个unsigned int保存4个字符信息,所以用128左移
    /*
    *添加原长度，长度指位的长度，所以要乘8，然后是小端序，所以放在倒数第二个,这里长度只用了32位
    */
    strByte[num*16-2]=str.length()*8;
    return strByte;
}

string MD5::changeHex(int a)
{
    int b;
    string str1;
    string str="";
    for(int i=0;i<4;i++)
    {
        str1="";
        b=((a>>i*8)%(1<<8))&0xff;   //逆序处理每个字节
        for (int j = 0; j < 2; j++)
        {
            str1.insert(0,1,str16[b%16]);
            b=b/16;
        }
        str+=str1;
    }
    return str;
}

string MD5::getMD5(string source)
{
    atemp=A;    //初始化
    btemp=B;
    ctemp=C;
    dtemp=D;
    unsigned int *strByte=add(source);
    for(unsigned int i=0;i<strlength/16;i++)
    {
        unsigned int num[16];
        for(unsigned int j=0;j<16;j++)
            num[j]=strByte[i*16+j];
        mainLoop(num);
    }
    return changeHex(atemp).append(changeHex(btemp)).append(changeHex(ctemp)).append(changeHex(dtemp));
}
