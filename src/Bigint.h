//
// Created by Lethe_k on 2019/12/8.
//

#ifndef UNTITLED1_BIGINT_H
#define UNTITLED1_BIGINT_H

#include <cstddef>  //  To use type : size_t
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

//question :  为何输入的字符串会少一截 ？
//question :  char buf[8] 就会崩掉？

using namespace std;
class Bigint
{
private:
    static const int BASE = 100000000;
    static const int BIT = 8;               // 8位一存
    bool sign;                              // plus = 1，minus = 0
    size_t length;                          // 位数
    vector <int> val;                       // 反存, vector从低位到高位存

    void CutLeadingZero();                  // 去掉多余的0
    Bigint abs() const;                     // 取绝对值
    Bigint power(const int &n) const;       // 乘10^n

public:
    // 构造函数
    explicit Bigint(int x = 0){val.clear(); length = 1; val.push_back(0); sign = 1;}         //默认构造函数
    explicit Bigint(const string &str);                                                      // string 类构造函数
    explicit Bigint(const bool &x){val.clear(); sign = 1 , length = 1, val.push_back(x);}
    explicit Bigint(const double &x);                                                        //double
    Bigint(const Bigint &B);                                                                 // 复制构造函数，

    //显式类型转化,// 不改变成员的值，为常量函数
    explicit operator double () const ;
    explicit operator string () const ;
    explicit operator bool () const ;

    //记录数的位数
    void SetLength() {
        CutLeadingZero();
        if (!val.back()) length = 1;
        else {
            length = (val.size() - 1) * BIT + 1;
            int tmp = val.back();
            while (tmp / 10) {
                length ++;
                tmp /= 10;
            }
        }
    }
    //判断是否为零
    bool is_Zero() const;

    // 重载为成员函数的运算符
    Bigint operator++();                       // 前置++
    Bigint operator++(int x);                  // 后置++
    Bigint operator--();                       // 前置--
    Bigint operator--(int x);                  // 后置--
    Bigint operator+=(const Bigint &B2);       // +=
    Bigint operator-=(const Bigint &B2);       // -=
    Bigint operator/=(const Bigint &B2);       // /=
    Bigint operator*=(const Bigint &B2);       // *=
    Bigint operator%=(const Bigint &B2);       // %=


//析构函数
~Bigint() {}

private:
    // Bigint
    friend Bigint operator+(const Bigint &B) ;                          // +
    friend Bigint operator+(const Bigint &B1, const Bigint &B2);        // +
    friend Bigint operator-(const Bigint &B);                           // -
    friend Bigint operator-(const Bigint &B1, const Bigint &B2);        // -
    friend Bigint operator*(const Bigint &B1, const Bigint &B2);        // *
    friend Bigint operator/(const Bigint &B1, const Bigint &B2);        // /
    friend Bigint operator%(const Bigint &B1, const Bigint &B2);        // %

    // bool
    friend bool operator==(const Bigint &B1, const Bigint &B2);         // ==
    friend bool operator!=(const Bigint &B1, const Bigint &B2);         // !=
    friend bool operator>(const Bigint &B1, const Bigint &B2);          // >
    friend bool operator<(const Bigint &B1, const Bigint &B2);          // <
    friend bool operator>=(const Bigint &B1, const Bigint &B2);         // >=
    friend bool operator<=(const Bigint &B1, const Bigint &B2);         // <=
    friend bool operator||(const Bigint &B1, const Bigint &B2);         // ||
    friend bool operator&&(const Bigint &B1, const Bigint &B2);         // &&
    friend bool operator!(const Bigint &B1);                            // !

    // << && >>
    friend ostream& operator<<(ostream &os, const Bigint &B);           // <<
    friend istream& operator>>(istream &is, Bigint &B);                 // >>  输入会改变对象的值，所以非常量引用

};


#endif //UNTITLED1_BIGINT_H
