//Bigint.cpp
//Created by Lethe_k on 2019/12/8.
//
#include "Bigint.h"
#include <iostream>
#include <algorithm>
#include <cstring>

void Bigint::CutLeadingZero() {
    while (val.back() == 0 && val.size() != 1)  val.pop_back();
}

bool Bigint::is_Zero() const {
    return !val.back();
}

Bigint::Bigint(const string &str) {
    //add something 
    char c[5000];
    string s1;
    strcpy(c, s1.c_str());
    if (s1.find(".") != std::string::npos)
    {
        double x = atof(c);
        *this = Bigint (x);
        SetLength();
    }
    //
    else{
        if (val.size() != 0) val.clear();
        if (str[0] == '-') {sign = 0; length = str.size() - 1;}
        else {sign = 1; length = str.size();}
        int num = ((int)str.size() - 1) / BIT + 1;
        int ans;
        for (int i = 0; i < num; i++) {     //num =2
            int tail = (int)str.length() - i * BIT;
            int head = max(tail - BIT, (int) (!sign));
            string x = str.substr(head, tail - head);
            ans = 0;
            int y = 1;
            for (int j = x.size() - 1; j >= 0; j--) {
                ans += y * ( x[j] - '0');
                y *= 10;
            }
            val.push_back(ans);
        }
        CutLeadingZero();
        SetLength();
    }
}

Bigint::Bigint(const Bigint &B) {
    if (val.size() != 0) val.clear();
    sign = B.sign;
    val = B.val;
    length = B.length;
}

Bigint::Bigint(const double &x) {
    if (val.size() != 0) val.clear();
    string str = to_string(x);
    for (int i = 0; i < 7; i++) {
        str.pop_back();
    }
    *this = (Bigint)(str);
    SetLength();
}

Bigint operator+(const Bigint &B) {
    return B;
}

Bigint operator+(const Bigint &B1, const Bigint &B2) {  //从头开始  8位8位的加
    int BASE = B1.BASE;
    if (B1.sign == 0) return B2 - (-B1);
    if (B2.sign == 0) return B1 - (-B2);
    Bigint ans ;
    Bigint tmp ;
    ans = (B1.val.size() > B2.val.size()) ? B1 : B2;
    tmp = (B1.val.size() > B2.val.size()) ? B2 : B1;
    if (B1.val.size() == B2.val.size())  ans.val.push_back(0);
    int carry = 0;
    int i;
    ans.sign = 1;
    for (i = 0; i < tmp.val.size(); i++)
    {
        int now;
        now = ans.val[i] + tmp.val[i] + carry;
        ans.val[i] = now % BASE;
        carry = now / BASE;
    }
    ans.val[i] += carry;
    if (ans.val.back() == 0)   ans.val.pop_back();
    ans.SetLength();
    return ans;
}

Bigint operator-(const Bigint &B) {
    Bigint ans = B;
    if (B.sign == 0) ans.sign = 1;
    else ans.sign = 0;
    return ans;
}

Bigint operator-(const Bigint &B1, const Bigint &B2) {
    if (!B2.sign) { return B1 + (-B2); }
    if (!B1.sign) {return -((-B1) + B2);}
    if (B1 < B2) {return -(B2 - B1); }
    Bigint ans;
    ans.sign = 1;
    ans.val.clear();
    for (int i = 0, g = 0; ; i++) {
        if (g == 0 && i >= B1.val.size() && i >= B2.val.size()) break;
        int x = g;
        g = 0;
        if (i < B1.val.size()) x += B1.val[i];
        if (i < B2.val.size()) x -= B2.val[i];
        if (x < 0) {
            x += B1.BASE;
            g = -1;
        }
        ans.val.push_back(x);
    }
    ans.SetLength();
    return ans;
}

Bigint operator*(const Bigint &B1, const Bigint &B2) {
    int BASE = B1.BASE;
    vector <long long >tmp;
    Bigint B_1 = B1.abs();
    Bigint B_2 = B2.abs();
    int lenB1 = B_1.val.size() , lenB2 =  B_2.val.size();
    if (tmp.size() != 0) tmp.clear();
    for (int i = 0; i < lenB1 + lenB2; i++) {tmp.push_back(0);}
    for (int i = 0; i < lenB1; i++) {
        for (int j = 0; j < lenB2; j++) {
            tmp[i + j] += (long long )B_1.val[i] * (long long) B_2.val[j];
        }
    }
    while (tmp.back() == 0 && tmp.size() != 1) tmp.pop_back();
    Bigint ans;
    if (ans.val.size() != 0) ans.val.clear();
    int len = tmp.size();
    long long int carry = 0, x;
    for (int i = 0; i < len; i++) {
        x = tmp[i];
        int y = (carry + x) % BASE;
        ans.val.push_back(y);
        carry = (x + carry) / BASE;
    }
    if(carry != 0)   ans.val.push_back(carry);
    if (B1.sign == B2.sign) ans.sign = 1;
    else ans.sign = 0;
    ans.SetLength();
    return ans;
}

Bigint operator/(const Bigint &B1, const Bigint &B2) {
    if (B2 == (Bigint)0)    exit(2);
    Bigint A = B1.abs();
    Bigint B = B2.abs();
    int la = A.length;
    int lb = B.length;
    if (A < B) return (Bigint)0;
    char str[5000];
    memset(str, 0, sizeof str);
    Bigint num;
    if (num.val.size() != 0)   num.val.clear();
    int i ;
    for (i = 0; i <= la - lb; i++) {
        num = B.power(la - lb - i);
        while (A >= num) {
            A = A - num;
            str[i]++;
        }
        str[i] +=  '0';
    }
    str[i] = '\0';
    Bigint ans((string)str);
    if (B1.sign == B2.sign) ans.sign = 1;
    else ans.sign = 0;
    return ans;
}

Bigint operator%(const Bigint &B1, const Bigint &B2) {
    return B1 - B1 / B2 * B2;
}

Bigint Bigint::abs() const{
    Bigint ans = *this;
    ans.sign = 1;
    return ans;
}

Bigint Bigint::power(const int &n) const {
    int x = n % BIT;                        //最后需要乘多少个10
    int y = n / BIT;                        //需要多少个vector
    Bigint ans;
    if (ans.val.size() != 0) ans.val.clear();
    for (int i = 0; i < y; i++)   ans.val.push_back(0);
    ans.val.push_back(1);
    for (int i = x; i > 0; i--) ans.val[y] *= 10;
    ans.length = n + 1;
    ans.sign = 1;
    return ans*(*this);
}

Bigint Bigint::operator++(){
    return *this + Bigint((double) 1);
}

Bigint Bigint::operator++(int x){
    Bigint ans = *this;
    *this += Bigint((double) 1);;
    return ans;
}

Bigint Bigint::operator--() {
    *this-= Bigint((double) 1);
    return *this;
}

Bigint Bigint::operator--(int x) {
    Bigint ans = *this;
    *this -= Bigint((double) 1);
    return ans;
}

Bigint Bigint::operator+=(const Bigint &B2) {
    *this = *this + B2;
    return *this;
}

Bigint Bigint::operator-=(const Bigint &B2) {
    *this = *this - B2;
    return *this;
}

Bigint Bigint::operator/=(const Bigint &B2) {
    *this = *this / B2;
    return *this;
}

Bigint Bigint::operator*=(const Bigint &B2) {
    *this = *this * B2;
    return *this;
}

Bigint Bigint::operator%=(const Bigint &B2) {
    *this = *this % B2;
    return *this;
}

bool operator>(const Bigint &B1, const Bigint &B2) {
    if (B1.sign > B2.sign) return true;
    if (B1.sign < B2.sign) return false;
    if ((B1.length > B2.length) && B1.sign > 0 && B2.sign > 0) return true;
    else if ((B1.length < B2.length) && B1.sign > 0 && B2.sign > 0) return false;
    if (B1.length < B2.length && B1.sign ==0 && B2.sign == 0) return true;
    else if ((B1.length > B2.length) && B1.sign == 0 && B2.sign == 0) return false;
    int i = B1.val.size() - 1;
    if(B1.sign == 1 && B2.sign == 1)
    {
        for (i; i >= 0; i--) {
            if (B1.val[i] > B2.val[i]) return true;
            else if (B1.val[i] < B2.val[i]) return false;
        }
        return false;
    }
    else 
    {
        for (i; i >= 0; i--) {
            if (B1.val[i] > B2.val[i]) return false;
            else if (B1.val[i] < B2.val[i]) return true;
        }
        return false;
    }
}

bool operator<(const Bigint &B1, const Bigint &B2) {
    return (B2 > B1);
}

bool operator!=(const Bigint &B1, const Bigint &B2) {
    if (B1 > B2) return true;
    return B1 < B2;
}

bool operator==(const Bigint &B1, const Bigint &B2) {
    return !(B1 != B2);
}

bool operator>=(const Bigint &B1, const Bigint &B2) {
    return !(B1 < B2);
}

bool operator<=(const Bigint &B1, const Bigint &B2) {
    return !(B1 > B2);
}

bool operator||(const Bigint &B1, const Bigint &B2) {
    if (B1 != Bigint((double) 0)) return true;
    return B2 != Bigint((double) 0);
}

bool operator&&(const Bigint &B1, const Bigint &B2) {
    return !(B2 == Bigint((double) 0) || B1 == Bigint((double) 0));
}

bool operator!(const Bigint &B1) {
    return B1 == Bigint((double) 0);
}

//
ostream &operator<<(ostream &os, const Bigint &B) {
    if (B.sign == 0) cout << '-';
    int num = B.val.size() - 2;
    os << B.val.back();
    for ( ; num >= 0; num--) {
        char *buf;
        asprintf(&buf, "%08d", B.val[num]);
        string str(buf);
        free(buf);
        cout << str;
    }
    return os;
}

istream &operator>>(istream &is, Bigint &B) {
    string str;
    is >> str;
    B = Bigint(str);
    return is;
}


// 显式类型转化  类 -> 系统内置
Bigint::operator double() const{
    double ans = 0;
    double x = sign ? 1 : -1;
    for (int i = 0; (i < val.size() && i < 2); i++) {
        ans += val[i] * x;
        x *= BASE;
    }
    return ans;
}

Bigint::operator string() const{
    char *str = new char [length + !sign];
    if (!sign) str[0] = '-';
    int cnt = !sign;
    for (int i = 0; i < val.size(); i++) {
        string tmp = to_string(val[i]);
        int x = tmp.length();
        for (int j = 0; j < x; j++) {
            str[cnt] = tmp[j];
            cnt ++;
        }
    }
    reverse(str + !sign, str + length + !sign);
    string ans(str, length + !sign);
    delete []str;
    return ans;
}

Bigint::operator bool() const{
    return !is_Zero();
}