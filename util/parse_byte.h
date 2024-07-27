
#ifndef PARSE_BYTE_H
#define PARSE_BYTE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <deque>
#include <sstream>

#include "Fbyte.h"
#include "json.hpp"
#include "Tvlog.h"

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

    // 通过命令分析数据的状态结构体，分析的结果是目标两个目标容器的累计
    struct ct_data
    {
        bool run = true;    // 运行标记，判断是否继续执行解析命令
        int skip = 0;       // 跳过命令，数量为跳过接来下的 N 条命令，用于执行分支命令
        std::deque<std::string> star;   // 标记容器，将需要标记的字符放入容器，用于对比是否执行部分命令
        std::deque<std::string> front;  // 头部容器，是目标容器，从剩余字符切割内容后放入目标容器，或称左端容器
        std::deque<std::string> back;   // 尾部容器，是目标容器，从剩余字符切割内容后放入目标容器，或称右端容器
        std::string data; // 剩余字符，命令主要针对剩余字符进行解析，解析的结果放入头或尾部容器，放入的目标容器是 * 号的同侧位置
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
        if(begin == "" || end == "") 
        {
            ret.push_back(str);
            return ret;
        }

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
            if(first == '!')
            {
                parse_number(data,cmd);
            }
            if(first == '#')
            {
                parse_node(data,cmd);
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

    static void parse_number(ct_data &data,std::string cmd)
    {
        if(find_exist(cmd,'('))
        {
            parse_number_offset(data,cmd);
        }
        else if(find_exist(cmd,'{'))
        {
            parse_number_find(data,cmd);
        }
        else 
        {
            parse_number_base(data,cmd);
        }
    }

    static void parse_number_base(ct_data &data,std::string cmd)
    {
        std::string slen = section_flg(cmd,'!','!');
        int len = from_string<int>(slen);

        auto tup_val = parse_format::carve_left(data.data,len);
        std::string sval = std::get<0>(tup_val);
        parse_number_op(data,cmd,sval);
    }

    static void parse_number_find(ct_data &data,std::string cmd)
    {
        std::string slen = section_flg(cmd,'!','!');
        int len = from_string<int>(slen);

        std::string sfind = section_flg(cmd,'{','}');

        if(find_exist(cmd,'*'))
        {
            less_star(sfind);
            auto tup_val = parse_format::find_str_sub(data.data,sfind);
            int ileft = std::get<0>(tup_val);
            std::string sval = std::string(data.data.begin(),data.data.begin() +ileft +1); 
            parse_number_op(data,cmd,sval);
        }
        else 
        {
            auto tup_val = parse_format::find_str_sub(data.data,sfind);
            int iright = std::get<0>(tup_val);
            std::string sval = std::string(data.data.begin() +iright +1,data.data.begin() +len +iright +1); 
            parse_number_op(data,cmd,sval);
        }

    }

    static void parse_number_offset(ct_data &data,std::string cmd)
    {
        std::string slen = section_flg(cmd,'!','!');
        int len = from_string<int>(slen);

        std::string soffset = section_flg(cmd,'(',')');
        int offset = from_string<int>(soffset);

        std::string sval = std::string(data.data.begin() +offset,data.data.begin() +offset +len); 
        parse_number_op(data,cmd,sval);
    }

    static void parse_number_op(ct_data &data,std::string cmd,std::string sval)
    {
        bool is_float = false;
        bool is_swap = false;
        bool is_unsigned = false;
        bool is_host_ip = false;
        std::string sflg = section_flg(cmd,'[',']');
        if(find_exist(sflg,'u'))
        {
            is_unsigned = true;
        }
        if(find_exist(sflg,'d'))
        {
            is_float = false;
        }
        if(find_exist(sflg,'f'))
        {
            is_float = true;
        }
        if(find_exist(sflg,'w'))
        {
            is_swap = true;
        }
        if(find_exist(sflg,'h'))
        {
            is_host_ip = true;
        }

        if(is_host_ip == true)
        {   
            std::string ret = to_host_ip(sval);
            data.front.push_back(ret);
            return;
        }

        std::string ret = hex_format_number(sval,is_float,is_swap,is_unsigned);
        if(is_float == false && is_swap == false && is_unsigned == false)
        {
            ret = "[s: " + ret + " ]";
        }
        else 
        {
            if(is_float)
            {
                ret = "[f: " + ret + " ]";
            }
            else 
            {
                ret = "[d: " + ret + " ]";
            }
        }
        data.front.push_back(ret);
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

    static void parse_node(ct_data &data,std::string cmd)
    {
        std::string node = section_flg(cmd,'#','#');
        node = "[n: " + node + " ]";
        data.front.push_back(node);
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


    static std::string hex_format_number(std::string val,bool is_float,bool is_swap,bool is_unsigned)
    {
        std::string ret;
        size_t len = val.size();

        if(is_float == false && is_swap == false && is_unsigned == false)
        {
            ret = Fbyte::sto_hex(val);
        }
        else 
        {
            if(is_float)
            {
                if(val.size() == 8)
                {
                    ret = Tto_num_endian<float>(val,is_swap);
                }
                if(val.size() == 16)
                {
                    ret = Tto_num_endian<double>(val,is_swap);
                }
            }
            else 
            {
                if(val.size() <= 4)
                {
                    if(is_unsigned)
                    {
                        ret = Tto_num_endian<unsigned short>(val,is_swap);
                    }
                    else 
                    {
                        ret = Tto_num_endian<short>(val,is_swap);
                    }
                }
                if(val.size() == 8)
                {
                    if(is_unsigned)
                    {
                        ret = Tto_num_endian<unsigned int>(val,is_swap);
                    }
                    else 
                    {
                        ret = Tto_num_endian<int>(val,is_swap);
                    }
                }
                if(val.size() == 16)
                {
                    if(is_unsigned)
                    {
                        ret = Tto_num_endian<unsigned long long>(val,is_swap);
                    }
                    else 
                    {
                        ret = Tto_num_endian<long long>(val,is_swap);
                    } 
                }
            }
        }
        return ret;
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

    static std::string to_host_ip(std::string shex)
    {
        std::vector<std::string> vec;
        for(int i=0;i<shex.size();i=i+2)
        {
            std::string sbyte;
            sbyte.push_back(shex[i]);
            sbyte.push_back(shex[i+1]);

		    std::string stwo = Fbyte::sto_hex(sbyte);
            unsigned short num = parse_cmd::Tmemcpy_num<unsigned short>(stwo);
            vec.push_back(std::to_string(num));
        }
        
        std::string ret;
        for(int i=0;i<vec.size() - 1;i++)
        {
            ret += vec[i] + ".";
        }
        ret += vec[vec.size() - 1];
        return ret;
    }

    template<typename T>
    static T from_string(const std::string& str)
    { 
        T t; 
        std::istringstream iss(str); 
        iss>>t; 
        return t;
    }

    template<typename T>
    static T Tmemcpy_num(const std::string& str)
    { 
        T t; 
        memcpy(&t,str.c_str(),sizeof(t));
        return t;
    }

    template<typename T>
    static std::string Tto_num_endian(const std::string& str,bool is_swap)
    { 
        std::string shex = Fbyte::sto_hex(str);
        T num = Tmemcpy_num<T>(shex);
        if(is_swap)
        {
            num = Fbyte::Tto_endian_host<T>(num);
        }
        return std::to_string(num);
    }
};

#endif // PARSE_BYTE_H
