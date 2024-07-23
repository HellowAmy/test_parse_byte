
#ifndef FBYTE_H
#define FBYTE_H

#include <iostream>
#include <cmath>
#include <cstring>
#include "Tvlog.h"

// == 大小端判断 ==
#ifndef BIG_ENDIAN
    #define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
    #define LITTLE_ENDIAN 1234
#endif
#ifndef NET_ENDIAN
    #define NET_ENDIAN BIG_ENDIAN
#endif

#ifndef BYTE_ORDER
    #if defined(__BYTE_ORDER)
        #define BYTE_ORDER  __BYTE_ORDER
    #elif defined(__BYTE_ORDER__)
        #define BYTE_ORDER  __BYTE_ORDER__
    #else
        #warning "Err Code Endian"
    #endif
#endif

#ifndef IS_BIG_ENDIAN
    #define IS_BIG_ENDIAN (BYTE_ORDER == BIG_ENDIAN)
#endif
#ifndef IS_LITTLE_ENDIAN
    #define IS_LITTLE_ENDIAN (BYTE_ORDER == LITTLE_ENDIAN)
#endif

#if IS_BIG_ENDIAN
    #define NOW_ENDIAN
#endif
#if IS_LITTLE_ENDIAN
    #define NOW_ENDIAN
#endif
// == 大小端判断 ==



struct Fbyte
{
    // 字符转字节,长度减半 ["ff" -> 0xff]
    static inline std::string sto_hex(const std::string &sbyte)
    {
        std::string ret;
        size_t len = sbyte.size() / 2;
        len *= 2;
        for(size_t i=0;i<len;i+=2)
        {
            ret.push_back(to_hex_byte(sbyte[i],sbyte[i+1]));
        }
        return ret;
    }

    // 字节转字符,长度加倍 [0xff -> "ff"]
    static inline std::string hto_hex(const std::string &sbyte)
    {
        std::string ret;
        for(int i=0;i<sbyte.size();i++)
        {
            ret += cto_shex(sbyte[i]);
        }
        return ret;
    }



    // 双字符合并为16进制字节
    static inline char to_hex_byte(char sleft,char sright)
    {
        char ret = 0;
        ret |= to_hex_left(sleft);
        ret |= to_hex_right(sright);
        return ret;
    }

    // 转8位二进制字符串-单字符
    static inline std::string to_sbyte(char c)
    {
        std::string ret;
        for(int i=0;i<8;i++)
        {
            if(c & 0x80)
            {
                ret.push_back('1');   
            }
            else 
            {
                ret.push_back('0');   
            }
            c <<= 1;
        }
        return ret;
    }

    // 转8位二进制字符串-字符串
    static inline std::string to_strbyte(const std::string &s,char flg = ' ')
    {
        std::string ret;
        for(int i=0;i<s.size();i++)
        {
            ret += to_sbyte(s[i]) + flg;
        }
        return ret;
    }
    
    // 单字节转双字符
    static inline std::string cto_shex(char c)
    {
        std::string ret;
        
        char t = 1;
        t <<= sizeof(t)*8 - 1;

        for(int byte=0;byte<2;byte++)
        {
            size_t num = 0;
            for(int i=0;i<4;i++)
            {
                if(t & c)
                {
                    num += pow(2,3-i);
                }
                c <<= 1;
            }
            if(num < 10)
            {
                ret.push_back(num + '0');
            }
            else 
            {
                num -= 10;
                ret.push_back(num + 'a');
            }
        }
        return ret;
    }    

    // ASCII码转16进制-右半段
    static inline char to_hex_right(char c)
    {
        char ret = 0;
        if(is_hex_digit(c))
        {
            ret = c - '0';
        }
        else if(is_hex_letter_little(c))
        {
            ret = c - 'a' + 10;
        }
        else if(is_hex_letter_big(c))
        {
            ret = c - 'A' + 10;
        }
        return ret;
    }

    // ASCII码转16进制-左半段
    static inline char to_hex_left(char c)
    {
        char ret = to_hex_right(c);
        ret <<= 4;
        return ret;
    }

    // 判断为十六进制数值范围
    static inline bool is_hex_range(char c)
    {
        if(is_hex_digit(c) || is_hex_letter_little(c) || is_hex_letter_big(c))
        {
            return true;
        }
        return false;
    }

    // 判断为数字
    static inline bool is_hex_digit(char c)
    {
        if((c >= '0' && c <= '9'))
        {
            return true;
        }
        return false;
    }

    // 判断为小写字母
    static inline bool is_hex_letter_little(char c)
    {
        if(c >= 'a' && c <= 'f')
        {
            return true;
        }
        return false;
    }

    // 判断为大写字母
    static inline bool is_hex_letter_big(char c)
    {
        if(c >= 'A' && c <= 'F')
        {
            return true;
        }
        return false;
    }

    // 转大写-单字符
    static inline char to_upper(char c)
    {
        const int w = 'a' - 'A';
        if(c >= 'a' && c <= 'z')
        {
            c -= w;
        }
        return c;
    }

    // 转小写-单字符
    static inline char to_lower(char c)
    {
        const int w = 'a' - 'A';
        if(c >= 'A' && c <= 'Z')
        {
            c += w;
        }
        return c;
    }

    // 转大写-字符串
    static inline std::string to_upper(const std::string &s)
    {
        std::string ret;
        ret.resize(s.size());
        
        for(int i=0;i<s.size();i++)
        {
            char c = to_upper(s[i]);
            ret[i] = c;
        }
        return ret;
    }

    // 转小写-字符串
    static inline std::string to_lower(const std::string &s)
    {
        std::string ret;
        ret.resize(s.size());
        
        for(int i=0;i<s.size();i++)
        {
            char c = to_lower(s[i]);
            ret[i] = c;
        }
        return ret;
    }

    // 取整数倍
    static inline size_t to_num_multi(size_t num,size_t multi)
    {
        num /= multi;
        num *= multi;
        return num;
    }



    // 转8位二进制字符串-未知长度
    template<typename T>
    static inline std::string Tto_sbyte(T c,char flg = ' ')
    {
        T t = 1;
        size_t len = sizeof(c)*8;
        t <<= len - 1;

        std::string ret;
        for(int i=0;i<len;i++)
        {
            if(c & t)
            {
                ret.push_back('1');   
            }
            else 
            {
                ret.push_back('0');   
            }
            c <<= 1;
            if((i+1)%8 == 0)
            {
                ret.push_back(flg);   
            }
        }
        return ret;
    }

    // 按字节顺序转8位二进制字符串-未知长度
    template<typename T>
    static inline std::string Tto_sbyte_endian(T c,char flg = ' ')
    {
        size_t len = sizeof(c);
        char arr[len];
        memcpy(arr,&c,len);

        char t = 1;
        t <<= 8 - 1;

        std::string ret;
        for(int i=0;i<len;i++)
        {
            char ic = arr[i];
            for(int x=0;x<8;x++)
            {
                if(ic & t)
                {
                    ret.push_back('1');   
                }
                else 
                {
                    ret.push_back('0');   
                }
                ic <<= 1;
                if((x+1)%8 == 0)
                {
                    ret.push_back(flg);   
                }
            }
        }
        return ret;
    }

    // 大小端字节序交换函数
    template<typename T>
    static inline T Tto_swap_endian(T val)
    {
        size_t len = sizeof(val);

        char arr[len];
        memcpy(arr,&val,len);

        for(int i=0;i<len;i++)
        {
            if(i >= len / 2)
            {
                break;
            }

            char tm = arr[i];
            arr[i] = arr[len -i -1];
            arr[len -i -1] = tm;
        }
        memcpy(&val,arr,len);
        return val;
    };

    // 转为主机序-传入为一定是网络大端
    template<typename T>
    static inline T Tto_endian_host(T val)
    {
        #if IS_BIG_ENDIAN
            return val;
        #else
            return Tto_swap_endian(val);
        #endif
    };

    // 转为网络序-传入不确定
    template<typename T>
    static inline T Tto_endian_net(T val)
    {
        #if IS_BIG_ENDIAN
            return val;
        #else
            return Tto_swap_endian(val);
        #endif
    };
};
#endif // FBYTE_H
