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

int main(int argc, char *argv[])
{
    std::cout<<"===== init log ====="<<std::endl;
	{
		Tvlogs::get()->set_level(vlevel4::e_info);
		Tflogs::get()->set_level(vlevel4::e_info);
		Tflogs::get()->init();
	}

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