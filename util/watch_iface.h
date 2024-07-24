#ifndef WATCH_IFACE_H
#define WATCH_IFACE_H

// #include <QObject>
// #include <QByteArray>

#include <stdio.h>
#include <pcap.h>
#include <iostream>
#include <vector>
#include <functional>
#include <thread>

#include "conf_file.h"
#include "Ttools.h"
#include "Tvlog.h"
#include "Fbyte.h"

using namespace Ttools;
using namespace net_ipv4;
using namespace std::placeholders;


// UDP包 14+20+8
struct pack_udp_t
{
    header_mac_t mac;
    header_ip_t ip;
    header_udp_t udp;
    std::string data;

    void update_len()
    {
        size_t tm_len_data = data.size();
		u_short tm_len_udp = tm_len_data + sizeof(header_udp_t);
		u_short tm_len_ip = tm_len_udp + sizeof(header_ip_t);
		ip.tlen = Fbyte::Tto_endian_net<u_short>(tm_len_ip);
		udp.length = Fbyte::Tto_endian_net<u_short>(tm_len_udp);
    }

    bool make_pack(std::vector<std::string> vec,bool is_update_len = true)
    {
        if(vec.size() != 4) return false;
        std::string tm_mac    = vec[0];
		std::string tm_ip     = vec[1];
		std::string tm_udp    = vec[2];
		std::string tm_data   = vec[3];
        data.resize(tm_data.size()/2);

        if(str_to_hex(tm_mac,(void*)&mac,sizeof(mac)) == false) return false;
        if(str_to_hex(tm_ip,(void*)&ip,sizeof(ip)) == false) return false;
        if(str_to_hex(tm_udp,(void*)&udp,sizeof(udp)) == false) return false;
        if(str_to_hex(tm_data,(void*)data.data(),data.size()) == false) return false;

        if(is_update_len) update_len();
        return true;
    }

    bool make_pack(std::string pack,bool is_update_len = true)
    {
        if(pack.size() < (sizeof(header_mac_t)+sizeof(header_ip_t)+sizeof(header_udp_t))) return false;

        size_t pos = 0;
        size_t len_ct = 0;

        len_ct = sizeof(header_mac_t);
        memcpy((void*)&mac,(void*)pack.data() + pos,len_ct);
        pos += len_ct;

        len_ct = sizeof(header_ip_t);
        memcpy((void*)&ip,(void*)pack.data() + pos,len_ct);
        pos += len_ct;

        len_ct = sizeof(header_udp_t);
        memcpy((void*)&udp,(void*)pack.data() + pos,len_ct);
        pos += len_ct;

        len_ct = pack.size() - pos;
        data.resize(len_ct);
        memcpy((void*)data.data(),(void*)pack.data() + pos,len_ct);
        pos += len_ct;

        if(is_update_len) update_len();
        return true;
    }

    std::string to_data()
    {
        size_t pos = 0;
        size_t len_ct = 0;
        char buf[sizeof(header_mac_t) + sizeof(header_ip_t) + sizeof(header_udp_t) + data.size()] = {0};
        char *pbuf = buf;

        len_ct = sizeof(header_mac_t);
        memcpy((void*)pbuf + pos,(void*)&mac,len_ct);
        pos += len_ct;

        len_ct = sizeof(header_ip_t);
        memcpy((void*)pbuf + pos,(void*)&ip,len_ct);
        pos += len_ct;

        len_ct = sizeof(header_udp_t);
        memcpy((void*)pbuf + pos,(void*)&udp,len_ct);
        pos += len_ct;

        len_ct = data.size();
        memcpy((void*)pbuf + pos,(void*)data.c_str(),len_ct);
        pos += len_ct;

        return std::string(buf,sizeof(buf));
    }
};


// 网卡监听类
class watch_iface_udp
{
public:
    watch_iface_udp()
    {
        // _filter = "ip and udp and host 10.1.0.2 and src 172.16.123.169";
        // _filter = "ip and udp and host 10.1.0.2 or host 224.0.2.4";
        _filter = _sp_conf_->filter_com;
    }

    ~watch_iface_udp()
    {
        _is_run = false;
        _run_th->join();
        delete _run_th;

        for(size_t i=0;i<_vec_pacp.size();i++)
        {
            pcap_t *adhandle = _vec_pacp[i];
            pcap_close(adhandle);
            adhandle = nullptr;
        }
        _vec_pacp.clear();
    }

    int init_watch()
    {
        char errbuf[PCAP_ERRBUF_SIZE];
        char buf[] = PCAP_SRC_IF_STRING;

        // 扫描网卡列表
        if (pcap_findalldevs_ex(buf, NULL, &_ls_iface, errbuf) == -1)
        {
            return -1;
        }

        // 打开所有网卡
        for(pcap_if_t *d=_ls_iface;d!=nullptr;d=d->next)
        {
            // 打开网卡
            pcap_t *adhandle = pcap_open_live(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 10, errbuf);
            if(adhandle == nullptr)
            {
                continue;
            }

            // 链接数据
            if(pcap_datalink(adhandle) != DLT_EN10MB)
            {
                continue;
            }

            // == 设置过滤器 ==
            u_int netmask;
            if(d->addresses != NULL)
            {
                netmask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.s_addr;
            }
            else
            {
                netmask=0xffffff;
            }

            struct bpf_program fcode;
            if (pcap_compile(adhandle, &fcode, _filter.c_str(), 1, netmask) < 0)
            {
                continue;
            }

            if (pcap_setfilter(adhandle, &fcode) < 0)
            {
                continue;
            }
            // == 设置过滤器 ==

            _vec_pacp.push_back(adhandle);
        }

        // 释放网卡列表
        pcap_freealldevs(_ls_iface);

        if(_vec_pacp.size() <= 0)
        {
            return -2;
        }

        return 0;
    }

    bool init_watch(pcap_t *adhandle)
    {
        if(adhandle != nullptr)
        {
            struct bpf_program fcode;
            if (pcap_compile(adhandle, &fcode, _filter.c_str(), 1, PCAP_NETMASK_UNKNOWN) >= 0 
                    && pcap_setfilter(adhandle, &fcode) >= 0)
            {
                _vec_pacp.push_back(adhandle);
                return true;
            }
        }
        return false;
    }


    void run_watch()
    {
        // 开始扫描网卡数据触发回调
        while(_is_run)
        {
            for(size_t i=0;i<_vec_pacp.size();i++)
            {
                pcap_t *adhandle = _vec_pacp[i];
                if(adhandle != nullptr)
                {
                    pcap_dispatch(adhandle, 1, &watch_iface_udp::scan_packet_cb, nullptr);
                }
            }
        }
    }

    void run_watch_th()
    {
        if(_run_th == nullptr)
        {
            _run_th = new std::thread(std::bind(&watch_iface_udp::run_watch,this));
        }
    }

    std::string get_filter_cmd()
    {
        return _filter;
    }

    static void scan_packet_cb(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
    {
        using namespace net_ipv4;
        size_t pos = 0;
        size_t len_ct = 0;

        pack_udp_t pudp;
        pudp.data = std::string((char*)pkt_data,header->len);
        
        if(_fn_data_cb) _fn_data_cb(&pudp);

        // if(pudp.make_pack(std::string((char*)pkt_data,header->len),false))
        // {
        //     if(_fn_data_cb) _fn_data_cb(&pudp);
        // }
        // else 
        // {
        //     floge($("scan_packet_cb parse failed"));
        // }
    }

public:
    static std::function<void (pack_udp_t *)> _fn_data_cb;

protected:
    bool _is_run = true;
    std::string _filter;
    std::thread *_run_th = nullptr;
    pcap_if_t *_ls_iface = nullptr;

    std::vector<pcap_t *> _vec_pacp;
};




#endif // WATCH_IFACE_H
