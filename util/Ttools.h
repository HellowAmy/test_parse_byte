
#ifndef TTOOLS_H
#define TTOOLS_H

#include <iostream>
#include <pcap.h>

// #include <QtEndian>
// #include <QByteArray>

#include "Fbyte.h"
#include "Tvlog.h"


namespace net_ipv4 {

// MAC长度 6
struct mac_addr_t
{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
    u_char byte5;
    u_char byte6;
};

// IP地址长度 4
struct ip_addr_t
{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
};

// MAC头 6+6+2=14
struct header_mac_t
{
    mac_addr_t src;
    mac_addr_t dst;
    u_short type;
};

// IP头 1+1+2+2+2+1+1+2+4+4=20
struct header_ip_t
{
    u_char ver_ihl;
    u_char tos;
    u_short tlen;
    u_short identification;
    u_short flags_fo;
    u_char ttl;
    u_char proto;
    u_short crc;
    ip_addr_t saddr;
    ip_addr_t daddr;
};

// UDP头 2+2+2+2=8
struct header_udp_t 
{
    u_short sport;
    u_short dport;
    u_short length;
    u_short check;
};

} // net_ipv4


namespace Ttools {

// 十六进制字符串转数组
static bool str_to_hex(const std::string &str,void *buf,size_t len)
{
    std::string arr = Fbyte::sto_hex(str);
    if(len == arr.size()) 
    {
        const char *p = arr.c_str();
        memcpy(buf,(void*)p,len);
        return true;
    }
    return false;
}

// 数组转十六进制字符
static std::string str_from_hex(void *buf,size_t len)
{
    std::string arr = Fbyte::hto_hex(std::string((char*)buf,len));
    if(len != arr.size()/2) return "";
    else return arr;
}

// == 大小端转换 ==
// template<class T>
// void to_big_data(void* dst, void *src)
// {
//     auto d = (T *)dst;
//     auto s = (T *)src;
//     *d = qToBigEndian<T>(*s);
// }

// template<class T>
// T to_big_data(void *src)
// {
//     auto s = (T *)src;
//     return qToBigEndian<T>(*s);
// }

// template<class T>
// void from_big_data(void* dst, void *src)
// {
//     auto d = (T *)dst;
//     auto s = (T *)src;
//     *d = qFromBigEndian<T>(*s);
// }

// template<class T>
// T from_big_data(void *src)
// {
//     auto s = (T *)src;
//     return qFromBigEndian<T>(*s);
// }


// // 验证主机通用IP地址
// static bool verity_general_ip(u_int ip)
// {
//     u_int ip_big;
//     to_big_data<u_int>(&ip_big,&ip);

//     net_ipv4::ip_addr_t ip_addr;
//     memcpy(&ip_addr,&ip_big,sizeof(ip_addr));
//     if(ip_addr.byte4 != 0x00 && ip_addr.byte4 != 0x01 && ip_addr.byte4 != 0xff && ip_addr.byte4 != 0xfe) return true;
//     else return false;
// }


// // == 快捷打印 ==
// static std::string byte_endian_type()
// {
//     union
//     {
//         int i;
//         char c;
//     }un;
//     un.i = 1;

//     if (un.c == 1) return "Endian: Little";
//     else return "Endian: Big";
// }
// // == 快捷打印 ==

} // Ttools



#endif // TTOOLS_H
