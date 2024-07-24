
#ifndef PARSE_BYTE_H
#define PARSE_BYTE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <deque>
#include <sstream>

#include "json.hpp"
// #include "Tvlog.h"

using njson = nlohmann::json;


struct parse_format
{
    static std::tuple<std::string,std::string> carve_left(std::string str,int index)
    {
        std::string sleft(str.begin(),str.begin() + index);
        std::string sright(str.begin() + index,str.end());
        return std::make_tuple(sleft,sright);
    }

    static std::tuple<std::string,std::string> carve_right(std::string str,int index)
    {
        std::string sleft(str.begin(),str.end() - index);
        std::string sright(str.end() - index,str.end());
        return std::make_tuple(sleft,sright);
    }

    static std::vector<std::string> carve_list_part(std::string str,int width)
    {
        std::vector<std::string> ret;
        std::string spart;
        for(int i=0;i<str.size();i++)
        {   
            spart.push_back(str[i]);
            if((i+1)% width == 0)
            {
                ret.push_back(spart);
                spart.clear();
            }
        }
        if(spart.size() > 0)
        {
            ret.push_back(spart);
        }
        return ret;
    }

    static std::vector<std::string> carve_list_parts(std::string str,std::vector<int> parts)
    {
        std::vector<std::string> ret;
        std::string spart;
        int count = 0;
        int width = parts[count];
        for(int i=0;i<str.size();i++)
        {   
            spart.push_back(str[i]);
            if((i+1)% width == 0)
            {
                count++;
                if(count >= parts.size())
                {
                    count = 0;
                }
                width = parts[count];
                ret.push_back(spart);
                spart.clear();
            }
        }
        if(spart.size() > 0)
        {
            ret.push_back(spart);
        }
        return ret;
    }

    static std::tuple<int,int> find_str_sub(std::string str,std::string sub)
    {
        int offset = 0;
        for(int i=0;i<str.size();i++)
        {
            if(str[i] == sub[offset])
            {
                offset++;
                if(offset >= sub.size())
                {
                    return std::make_tuple(i -sub.size() +1,i);
                }
            }
            else 
            {
                offset = 0;
            }
        }
        return std::make_tuple(-1,-1);
    }
};


//! 功能: 格式化十六进制字符流包体
//! 逻辑: 主要用于切割字符串，每次切割之后会得到目标字符串和剩余字符串，由左右方向的目标容器记录目标字符串，
//!         "*"号在左侧，则代表剩余字符串在左侧，目标字符串会被放入右侧容器
//! 命令: 
//!     "{ss}*"                 : 查找指定字符，查找到 ss 字符并切割，将剩余字符串放入右侧
//!     "*{ss}"                 : 查找指定字符，查找到 ss 字符并切割，将剩余字符串放入左侧
//!     "(dd)*"                 : 偏移指定位置，从剩余字符偏移 dd 位数值，切割并将剩余字符放入放入右侧
//!     "*(dd)"                 : 偏移指定位置，从剩余字符偏移 dd 位数值，切割并将剩余字符放入放入左侧
//!     "<dd>=dd"               : 记录长度字符，从剩余字符记录 dd 位数值到标记容器，不切割且剩余字符不变
//!     "#[+dd+dd]"             : 格式数据列表，从剩余字符循环切割多个不同的 dd 数值放入左侧目标容器，直到剩余字符为空
//!     "#(dd){ss}[+dd+dd]"     : 条件格式列表，查看 dd 位标记容器是否为 ss ，如果一致则格式化列表，否则跳过该条命令
//! 
struct parse_cmd
{
    enum en_status
    {
        e_append_before,
        e_appending,
        e_append_after,
    };

    enum en_dire
    {
        e_null,
        e_left,
        e_right,
    };

    struct ct_save_index
    {
        std::string val;
        int index;
    };

    struct ct_format_list
    {
        std::vector<int> list;
        ct_save_index index;
    };

    struct ct_data
    {
        bool run = true;
        int skip = 0;
        std::deque<std::string> star;
        std::deque<std::string> front;
        std::deque<std::string> back;
        std::string data;
    };

    static std::vector<std::string> parse_pack(std::string str,std::vector<std::string> cmds,std::string begin,std::string end)
    {
        std::vector<std::string> ret;
        auto arr = parse_cmd::parse_data_arr(str,begin,end);
        for(auto &a : arr)
        {
            auto vec = parse_cmd::parse_cmds(a,cmds);
            for(auto &b : vec)
            {
                ret.push_back(b);
            }
        }
        return ret;
    }

    static std::vector<std::string> parse_data_arr(std::string str,std::string begin,std::string end)
    {
        std::vector<std::string> ret;

        while(true)
        {
            auto tibegin = parse_format::find_str_sub(str,begin);
            auto tiend = parse_format::find_str_sub(str,end);
            int ibegin = std::get<0>(tibegin);
            int iend = std::get<1>(tiend)+1;

            if(ibegin == -1 || iend == -1)
            {
                break;
            }

            ret.push_back(std::string(str.begin()+ibegin,str.begin()+iend));
            str = std::string(str.begin()+iend,str.end());
        }

        return ret;
    }

    static std::vector<std::string> parse_cmds(std::string str,std::vector<std::string> cmds)
    {
        std::vector<std::string> ret;
        ct_data data;
        data.data = str;

        for(auto cmd : cmds)
        {
            if(data.run == false)
            {
                break;
            }
            if(data.skip > 0)
            {
                data.skip--;
                continue;
            }

            char first = cmd[0];
            if(first == '{')
            {
                parse_sp(data,cmd);
            }
            if(first == '(')
            {
                parse_offset(data,cmd);
            }
            if(first == '<')
            {
                parse_star(data,cmd);
            }
            if(first == '[')
            {
                parse_list(data,cmd);
            }
            if(first == ':')
            {
                parse_skip_cond(data,cmd);
            }
        }

        for(auto a: data.front)
        {
            ret.push_back(a);
        }
        for(auto a: data.back)
        {
            ret.push_back(a);
        }

        return ret;
    }

    static void parse_sp(ct_data &data,std::string cmd)
    {
        en_dire ed = find_star_dire(cmd);
        std::string sub = section_flg(cmd,'{','}');
        less_star(sub);

        std::tuple<int,int> offset = parse_format::find_str_sub(data.data,sub);
        int ileft = std::get<0>(offset);
        int iright = std::get<1>(offset);

        if(ed == e_right)
        {
            auto tup = parse_format::carve_left(data.data,iright + 1);
            data.front.push_back(std::get<0>(tup));
            data.data = std::get<1>(tup);
        }
        else if(ed == e_left)
        {
            auto tup = parse_format::carve_left(data.data,ileft);
            data.back.push_front(std::get<1>(tup));
            data.data = std::get<0>(tup);
        }
    }

    static void parse_offset(ct_data &data,std::string cmd)
    {
        en_dire ed = find_star_dire(cmd);
        std::string sub = section_flg(cmd,'(',')');
        less_star(sub);
        int val = from_string<int>(sub);

        if(ed == e_right)
        {
            auto tup = parse_format::carve_left(data.data,val);
            data.front.push_back(std::get<0>(tup));
            data.data = std::get<1>(tup);
        }
        else if(ed == e_left)
        {
            auto tup = parse_format::carve_right(data.data,val);
            data.back.push_front(std::get<1>(tup));
            data.data = std::get<0>(tup);
        }
    }

    static void parse_skip_cond(ct_data &data,std::string cmd)
    {
        std::string snum = section_flg(cmd,':',':');
        std::string sindex = section_flg(cmd,'(',')');
        std::string sdata = section_flg(cmd,'{','}');

        int index = from_string<int>(sindex);
        
        if(data.star.size() > index)
        {
            if(data.star[index] != sdata)
            {
                data.skip = from_string<int>(snum);
            }
        }
    }

    static void parse_star(ct_data &data,std::string cmd)
    {
        if(find_exist(cmd,'('))
        {
            parse_star_offset(data,cmd);
        }
        else if(find_exist(cmd,'{'))
        {
            parse_star_find(data,cmd);
        }
        else 
        {
            parse_star_base(data,cmd);
        }

    }

    static void parse_star_base(ct_data &data,std::string cmd)
    {
        std::string snum = section_flg(cmd,'<','>');
        int num = from_string<int>(snum);

        auto tup = parse_format::carve_left(data.data,num);
        int index = equal_num(cmd);
        add_star(data,index,std::get<0>(tup));
    }

    static void parse_star_find(ct_data &data,std::string cmd)
    {
        std::string snum = section_flg(cmd,'<','>');
        int num = from_string<int>(snum);

        std::string sfind = section_flg(cmd,'{','}');
        std::tuple<int,int> offset = parse_format::find_str_sub(data.data,sfind);
        int ileft = std::get<0>(offset);
        int iright = std::get<1>(offset) + 1;

        auto tup = parse_format::carve_left(std::string(data.data.begin() + iright,data.data.end()),num);
        int index = equal_num(cmd);
        add_star(data,index,std::get<0>(tup));
    }

    static void parse_star_offset(ct_data &data,std::string cmd)
    {
        std::string snum = section_flg(cmd,'<','>');
        int num = from_string<int>(snum);

        std::string soffset = section_flg(cmd,'(',')');
        int offset = from_string<int>(soffset);

        auto tup = parse_format::carve_left(std::string(data.data.begin() + offset,data.data.end()) ,num);
        int index = equal_num(cmd);
        add_star(data,index,std::get<0>(tup));
    }

    static void parse_list(ct_data &data,std::string cmd)
    {
        if(find_exist(cmd,'('))
        {
            parse_list_select(data,cmd);
        }
        else 
        {
            parse_list_empty(data,cmd);
        }
    }

    static void parse_list_select(ct_data &data,std::string cmd)
    {
        std::string snum = section_flg(cmd,'(',')');
        int num = from_string<int>(snum);

        std::string sflg = section_flg(cmd,'{','}');

        if(data.star.size() > num)
        {
            if(data.star[num] == sflg)
            {
                parse_list_empty(data,cmd);
            }
        }
    }

    static void parse_list_empty(ct_data &data,std::string cmd)
    {
        data.run = false;
        std::string slist = section_flg(cmd,'[',']');
        std::vector<std::string> vec = section_flg(slist,'+');
        parse_list_format(data,vec);

        if(data.data != "")
        {
            data.front.push_back(data.data);
            data.data.clear();
        }
    }

    static void parse_list_format(ct_data &data,std::vector<std::string> vec)
    {
        std::vector<int> ivec;
        for(auto a:vec)
        {
            int num = from_string<int>(a);
            ivec.push_back(num);
        }
        if(ivec.size() == 0)
        {
            return;
        }

        int index = 0;
        while(true)
        {
            if(index >= ivec.size())
            {
                index = 0;
            }
            if(data.data == "" || data.data.size() < ivec[index])
            {
                break;
            }

            auto tup = parse_format::carve_left(data.data,ivec[index]);
            data.front.push_back(std::get<0>(tup));
            data.data = std::get<1>(tup);
            index++;
        }
    }



    static en_dire find_star_dire(std::string str)
    {
        en_dire ed;
        if(str[1] == '*')
        {
            ed = e_left;
        }
        else if(str[str.size() - 2] == '*')
        {
            ed = e_right;
        }
        return ed;
    }

    static std::string section_flg(std::string str,char fbegin,char fend)
    {
        int ibegin = -1;
        int iend = -1;
        for(int i=0;i<str.size();i++)
        {
            char c = str[i];
            if(fbegin == c && ibegin == -1)
            {
                ibegin = i;
            }
            else if(c == fend && iend == -1)
            {
                iend = i;
            }
        }
        if(ibegin != -1 && iend != -1)
        {
            return std::string(str.begin() + ibegin + 1,str.begin() + iend);
        }
        return "";
    }

    static void less_star(std::string &str)
    {
        for(int i=0;i<str.size();i++)
        {
            char c = str[i];
            if(c == '*')
            {
                str.erase(i,1);
                return ;
            }
        }
    }

    static void add_star(ct_data &data,int index,std::string val)
    {
        if(data.star.size() <= index)
        {
            data.star.resize(index + 1);
        }
        data.star[index] = val;
    }

    static int equal_num(std::string &str)
    {
        for(int i=0;i<str.size();i++)
        {
            char c = str[i];
            if(c == '=')
            {
                return from_string<int>(std::string(str.begin() +i +1,str.end()));
            }
        }
        return -1;
    }

    static std::vector<std::string> section_flg(std::string str,char flg,bool skip_empty = true)
    {
        std::vector<std::string> ret;
        std::string ele;
        for(int i=0;i<str.size();i++)
        {
            char c = str[i];
            if(c == flg)
            {
                if(skip_empty)
                {
                    if(ele != "")
                    {
                        ret.push_back(ele);
                    }
                }
                else 
                {
                    ret.push_back(ele);
                }
                ele.clear();
            }
            else 
            {
                ele.push_back(c);
            }
        }
        if(ele.size() > 0)
        {
            ret.push_back(ele);
        }

        return ret;
    }

    static bool find_exist(std::string cmd,char c)
    {
        for(auto &a:cmd)
        {
            if(a == c)
            {
                return true;
            }
        }
        return false;
    }


    template<typename T>
    static T from_string(const std::string& str)
    { 
        T t; 
        std::istringstream iss(str); 
        iss>>t; 
        return t;
    }

};

#endif // PARSE_BYTE_H
