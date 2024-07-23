
#ifndef IFAC_SOCK_H
#define IFAC_SOCK_H

// #include <QByteArray>
#include <iostream>
#include <vector>
#include <pcap.h>

#include "Fbyte.h"
#include "Tvlog.h"

class ifac_sock
{
public:
    ifac_sock(){}
    ~ifac_sock() 
    {
        if(_ifac_dev != nullptr) pcap_close(_ifac_dev);
    }

    void scan_ifac_dev()
    {
        char buf[1024];
        pcap_if_t *pall;
        pcap_findalldevs(&pall, buf);
        for (pcap_if_t *pdev = pall; pdev; pdev=pdev->next)
        {
            // vlogd($(pdev->name) $(pdev->description) $(pdev->flags));
            _vec_ifac_id.push_back(pdev->name);
        }
        pcap_freealldevs(pall);
    }

    bool send_data_pack(u_char *pack,size_t len)
    {
        if(_ifac_dev == nullptr) 
        {
            floge("_ifac_dev is null"<< $(Fbyte::hto_hex(std::string((char*)pack,len))));
            return false;
        }
        if(pcap_sendpacket(_ifac_dev,pack,len) != 0)
        {
            floge("pcap_sendpacket send failed"<< $(Fbyte::hto_hex(std::string((char*)pack,len))));
            return false;
        }
        return true;
    }

    bool open_ifac(std::string name)
    {
        _ifac_name = name;
        _ifac_dev = pcap_open_live(_ifac_name.c_str(),65536,PCAP_OPENFLAG_PROMISCUOUS,10,nullptr);
        return _ifac_dev != nullptr;
    }

    std::string get_ifac_name()
    {
        return _ifac_name;
    }

    pcap_t* get_ifac_dev()
    {
        return _ifac_dev;
    }

    std::vector<std::string> get_ifac_id()
    {
        return _vec_ifac_id;
    }

protected:

private:
    std::string _ifac_name;
    pcap_t *_ifac_dev = NULL;
    std::vector<std::string> _vec_ifac_id;
};

static ifac_sock *_sp_ifac_sock_ = Tsingle_d<ifac_sock>::get();


#endif // IFAC_SOCK_H
