#include <iostream>
#include <deque>
#include <thread>

#include "parse_byte.h"

#include "Tvlog.h"
#include "watch_iface.h"
#include "ifac_sock.h"
#include "conf_file.h"
#include "Ttools.h"
#include "handle_gloabl.h"

using namespace net_ipv4;
using namespace Ttools;
using namespace std;

std::function<void (pack_udp_t *)> watch_iface_udp::_fn_data_cb = nullptr;



void test_1()
{
	std::string ss = "aa123456789bb";
	std::vector<std::string> vec {
		"<4>=0",
		"<4>(5)=1",
		"<4>{45}=2",
	};
	parse_cmd::parse_cmds(ss,vec);
}

void test_2()
{
	std::string ss = "aaee1ffff0800490000000424c03000100000f9090000004c1b8c1b8c1b8c1b89a9deefc";
	std::vector<std::string> vec {
		"{aaee}*",
        "*{eefc}",
        "{424c}*",
        "(14)*",
        "<4>=0",
        "(8)*",
        "*(4)",
        "#(0){9000}[+1+3]",
        "#(0){9090}[+5]",
        "#[+4]",
	};
	auto ret = parse_cmd::parse_cmds(ss,vec);
	print_con(ret,1);
/*
| size: 12
| aaee
| 1ffff0800490000000424c
| 03000100000f90
| 90000004
| c
| 1b8
| c
| 1b8
| c
| 1b8
| c
| 1b8
| size: 2
| 9a9d
| eefc
| size: 1
| 9000
| size: 14
| aaee
| 1ffff0800490000000424c
| 03000100000f90
| 90000004
| c
| 1b8
| c
| 1b8
| c
| 1b8
| c
| 1b8
| 9a9d
| eefc
*/
}

void test_3()
{
	std::string ss = "aaee1ffff0800490000000424c03000100000f9090000004c1b8c1b8c1b8c1b89a9deefc";
	std::vector<std::string> vec {
        "<4>{ffff}=0",
		"{aaee*}",
        "{*eefc}",
        "{424c*}",
        "(14*)",
        "<4>=1",
        "<4>(4)=2",
        "(8*)",
        "(*4)",
        "[+1+3](1){9000}",
        "[+5](1){9090}",
        "[+4]",
	};
	auto ret = parse_cmd::parse_cmds(ss,vec);
	print_con(ret,1);
}
void test_4()
{
	std::string s1 = "aaee*";
	std::string s2 = "*eefc";

	s1 = s1.erase(4,1);
	vlogd($(s1));

	s2 = s2.erase(0,1);
	vlogd($(s2));
}


int main(int argc, char *argv[])
{
    std::cout<<"===== init log ====="<<std::endl;
	{
		Tvlogs::get()->set_level(vlevel4::e_info);
		Tflogs::get()->set_level(vlevel4::e_info);
		Tflogs::get()->init();
	}


#if 0
	// test_1();
	// test_2();
	test_3();
	// test_4();
	return 0;
#endif


	std::cout<<"===== init config ====="<<std::endl;
	{
		bool load_config_json = _sp_conf_->parse_sjson("../config/config.json");
		vlogd($(load_config_json));

		bool load_sp_template_ = _sp_template_->parse_sjson("../config/template.json");
		vlogd($(load_sp_template_));
	}

    std::cout<<"===== init watch ====="<<std::endl;
	{
		_sp_ifac_sock_->scan_ifac_dev();
		auto vec = _sp_ifac_sock_->get_ifac_id();

		std::cout<<"请选择网卡："<<std::endl;
		print_con(vec,1);

		std::string ifac_name;
		if(_sp_conf_->ifac_index == "")
		{
			std::string str_ifac;
			std::cin >> str_ifac;
			int index = parse_cmd::from_string<int>(str_ifac);
			ifac_name = vec[index];
		}
		else 
		{
			int index = parse_cmd::from_string<int>(_sp_conf_->ifac_index);
			ifac_name = vec[index];
		}

		bool open_ifac_succeed = _sp_ifac_sock_->open_ifac(ifac_name);
		vlogd($(ifac_name) $(open_ifac_succeed));
	}

	{
		std::cout<<"请选择格式化模式："<<std::endl;
		for(auto a:_sp_template_->mode_list)
		{
			print_con(a.format_cmds,1);
		}

		int index = 0;
		if(_sp_conf_->mode_index == "")
		{
			std::string str_ifac;
			std::cin >> str_ifac;
			index = parse_cmd::from_string<int>(str_ifac);
		}
		else 
		{
			index = parse_cmd::from_string<int>(_sp_conf_->mode_index);
		}

		_sp_template_->set_index(index);
		vlogd($(index));
	}

	watch_iface_udp udp;
    udp._fn_data_cb = [](pack_udp_t *pack){
		auto shex = Fbyte::hto_hex(pack->data);
        vlogd($(shex));
		auto vec = parse_cmd::parse_pack(shex,
								_sp_template_->get_data().format_cmds,
								_sp_template_->get_data().pack_begin,
								_sp_template_->get_data().pack_end
							);
		print_con(vec,1);
    };
	bool init_watch_succeed = udp.init_watch(_sp_ifac_sock_->get_ifac_dev());
	vlogd($(init_watch_succeed));
    udp.run_watch_th();

	std::cout<<"输入任意字符后退出："<<std::endl;
    while(true)
    {
		std::string str_exit;
		std::cin >> str_exit;
		if(str_exit != "")
		{
			break;
		}
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout<<"== 程序退出 =="<<std::endl;
    return 0;
};

/*
切割进左，
切割进右，
查找切割，
偏移切割，
循环格式化，
条件格式化，
标记长度字符，
标记偏移长度，
标记查找长度，
*/