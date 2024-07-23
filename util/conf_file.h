
#ifndef CONF_FILE_H
#define CONF_FILE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "Tvlog.h"
#include "json.hpp"

struct conf_file
{
    using njson = nlohmann::json;

    enum en_recv_protocol
    {
        e_dev_up,
        e_dev_reload,
        e_keep_normal,
        e_keep_sign,
        e_keep_vote,
        e_mic_open,
        e_mic_close,
        e_sign_in,
        e_sign_out,
        e_null,
    };

    bool parse_sjson(std::string s)
    {
        _filename = s;
        try {
            std::ifstream fs(s);
            _json = njson::parse(fs);

            filter_com = _json["filter_com"];
            ifac_name = _json["ifac_name"];
            ifac_index = _json["ifac_index"];
            mode_index = _json["mode_index"];
            return true;
        }
        catch(...){}
        return false;
    }

    void write_ifac_name(std::string s)
    {
        _json["ifac_name"] = s;
        ifac_name = s;
    }

    void save_sjson()
    {
        std::ofstream fs(_filename);
        fs << std::setw(4) << _json << std::endl;
    }

    njson _json;
    std::string _filename;
    std::string filter_com;
    std::string ifac_name;
    std::string ifac_index;
    std::string mode_index;
};

struct conf_template
{
    using njson = nlohmann::json;

    struct mode_list_ct
    {
        std::string pack_begin;
        std::string pack_end;
        std::vector<std::string> format_cmds;
    };

    bool parse_sjson(std::string s)
    {
        _filename = s;
        try {
            std::ifstream fs(s);
            _json = njson::parse(fs);

            for(int i=0;i<_json["mode_list"].size();i++)
            {
                mode_list_ct it;
                it.pack_begin = _json["mode_list"][i]["pack_begin"];
                it.pack_end = _json["mode_list"][i]["pack_end"];

                std::vector<std::string> format_cmds;
                for(int x=0;x<_json["mode_list"][i]["format_cmds"].size();x++)
                {
                    format_cmds.push_back(_json["mode_list"][i]["format_cmds"][x]);
                }
                it.format_cmds = format_cmds;
                mode_list.push_back(it);
            }
            return true;
        }
        catch(...){}
        return false;
    }

    mode_list_ct get_data()
    {
        return mode_list[_index];
    }

    void set_index(int index)
    {
        _index = index;
    }

    njson _json;
    std::string _filename;
    int _index = 0;
    std::vector<mode_list_ct> mode_list;
};

static conf_file *_sp_conf_ = Tsingle_d<conf_file>::get();
static conf_template *_sp_template_ = Tsingle_d<conf_template>::get();

#endif // CONF_FILE_H
