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
		":1:(0){9000}",
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

void test_5()
{
	std::string ss = "aaee1ffff0800490000000424c03000100000f9090900004c1b8c1b8c1b8c1b89a9deefc";
	std::vector<std::string> vec {
		"{aaee*}",
        "{*eefc}",
        "{424c*}",
        "(14*)",
        "<4>=1",
        "(8*)",
        "(*4)",
		":1:(0){9000}",
        "[+1+3]",
		":1:(0){9090}",
        "[+5]",
        "[+4]",
	};
	auto ret = parse_cmd::parse_cmds(ss,vec);
	print_con(ret,1);
}

void test_6()
{
	std::string ss = "aaee1ffff0800490000000424c03000100000f9090900004c1b8c1b8c1b8c1b89a9deefc";
	ss += "aaee1ffff0800490000000424c03000100000f9090000004c1b8c1b8c1b8c1b89a9deefc";
	std::vector<std::string> vec {
        "<4>(22)=1",
		"{aaee*}",
        "{*eefc}",
        "{424c*}",
        "(14*)",
        "<4>=0",
        "(8*)",
        "(*4)",
		":1:(0){9000}",
        "[+1+3]",
		":1:(0){9090}",
        "[+5]",
        "[+4]",
	};
	auto ret = parse_cmd::parse_pack(ss,vec,"aaee","eefc");
	print_con(ret,1);
}

void test_7()
{
	std::string snum = "c421";
	std::string hex = Fbyte::sto_hex(snum);
	unsigned short num;
	memcpy(&num,hex.c_str(),sizeof(num));
	unsigned short hnum = Fbyte::Tto_endian_host<short>(num);
	vlogd($(num) $(hnum));
}

void test_8()
{
	{
		std::string ss = "aaee1ffff0800490000000424c03000100000f9090000004c1b8c1b8c1b8c1b89a9deefc";
		std::vector<std::string> vec {
			"{aaee*}",
			"{*eefc}",
			"{424c*}",
			"(14*)",
			"!4![u:d:w]",
			"!8!(4)[u:f:d:w]",
			"!16!{0004}[u:f:d:w]",
			"<4>=1",
			"(8*)",
			":1:(1){9000}",
			"#这是注释#",
			"(*4)",
			":1:(0){9000}",
			"[+1+3]",
			":1:(0){9090}",
			"[+5]",
			"[+4]",
		};
		
		auto ret = parse_cmd::parse_cmds(ss,vec);
		print_con(ret,1);
	}


	{
		std::string ss = "aaee485454502f312e3120323030204f4b0d0a5365727665723a206e67696e780d0a446174653a204672692c203236204a756c20323032342030333a32333a313820474d540d0aeefc";
		std::vector<std::string> vec {
			"(4*)",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
		};
	
		auto ret = parse_cmd::parse_cmds(ss,vec);
		print_con(ret,1);
	}
}

void test_9()
{
	{
		bool is_float =false;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "9000";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float =false;
		bool is_swap = false;
		bool is_unsigned = true;
		std::string ss = "90";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float =false;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "0090";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float =false;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "90004456";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "4149374B";// 浮点数大小端问题：所有字节反转，小数字节再次反转
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = false;
		bool is_unsigned = true;
		std::string ss = "4B374941";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = false;
		bool is_unsigned = true;
		std::string ss = "4B3749414B374941";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "4B3749414B374941";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = true;
		bool is_unsigned = true;
		std::string ss = "a27c41cb6f30fe40";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = true;
		bool is_swap = false;
		bool is_unsigned = true;
		std::string ss = "a27c41cb6f30fe40";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		double bnum = 123654.987123;
		char buf[8] = {0};
		memcpy(buf,&bnum,sizeof(bnum));
		auto s1 = Fbyte::hto_hex(std::string(buf,sizeof(buf)));
		vlogd($(s1)); // a27c41cb6f30fe40
	}

}

void test_10()
{
	{
		std::string ss = "485454502f312e31";
		std::string hex = Fbyte::sto_hex(ss);
		vlogd($(hex));
	}
	{
		std::string ss = "323030";
		std::string hex = Fbyte::sto_hex(ss);
		vlogd($(hex));
	}
	{
		bool is_float = false;
		bool is_swap = false;
		bool is_unsigned = false;
		std::string ss = "485454502f312e31";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		bool is_float = false;
		bool is_swap = false;
		bool is_unsigned = false;
		std::string ss = "3134610d0a";
		std::string ret = parse_cmd::hex_format_number(ss,is_float,is_swap,is_unsigned);
		vlogd($(ret));
	}
	{
		std::string str = "b4";
		std::string shex = Fbyte::sto_hex(str);
        unsigned short num = parse_cmd::Tmemcpy_num<unsigned short>(shex);
		vlogd($(num)); 

		// b4a3ebab  180.163.235.171
		// ac10158c  172.16.21.140
	}





	{
		std::string str = "b4a3ebab";
		std::string w = parse_cmd::to_host_ip(str);
		vlogd($(w)); 
	}
	{
		std::string str = "ac10158c";
		std::string w = parse_cmd::to_host_ip(str);
		vlogd($(w)); 
	}
}

void test_11()
{
	{
		std::string ss = "00e04c320a07e8a66099fc77080045000247bdbd400033062608b4a3ebabac10158c0050c96c59fa291dc827b264501801f5329b0000485454502f312e3120323030204f4b0d0a5365727665723a206e67696e780d0a446174653a204672692c203236204a756c20323032342030333a32333a313820474d540d0a436f6e74656e742d547970653a206170706c69636174696f6e2f6f637465742d73747265616d0d0a5472616e736665722d456e636f64696e673a206368756e6b65640d0a436f6e6e656374696f6e3a20636c6f73650d0a43616368652d436f6e74726f6c3a206e6f2d63616368650d0a707261676d613a206e6f2d63616368650d0a0d0a3134610d0a0a040140613700010000110250833bb629eae348cb7560c8fbcbaa447c6cc1ee54ed932b1e3d7509bb8cb65262812dcce23d9c1b9a1f20516e176b8cb1d538a7e8843a5c33bc3bf6248cfd1a20001ef57c15235ba6d7c932d64d71c406069b079f3cff52a70ba00fa004aa06a650f654a601a506a703f303f600a606f057a604a101a103f403b952a400a353f457a107a00fa052a507aa54f60ff10ef704a555f601f706a004f40ea50ea705f653a71da605a705a70fab06f657a301a501a452a705f306ab03aa54aa07a154a300a406a650f60fa654a750f101f357a102f150a055a200a106f404a404a657ab01a4559b07a218a23fa23fa23fa23fa305a6029b009b3f9b66da78e46cfc64e855ff7bb955a07fa47bf877e57bd677e57bd663eb79da4eaa55a07ce779e753db45f757de5cc641e455a00fff52da78eb6fe802af3f9b069b069b3c9f3c0d0a300d0a0d0a";
		std::vector<std::string> vec {
			"(24*)",
			"<4>=0",
			":1:(0){0800}",
			"#这是IP协议#",
			"#这是包体总长度#",
			"!4!(8)[u:d:w]",
			"(22*)",
			"<2>=1",
			":1:(1){06}",
			"#这是TCP协议#",
			"!2![u:d]",
			"(6*)",
			"#源地址#",
			"!8![h]",
			"(8*)",
			"#目标地址#",
			"!8![h]",
			"(8*)",
			"(40*)",
			"!34![]",
			"#Http协议返回值#",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"!0!{*0d0a}[]",
			"{0d0a*}",
			"[+10000]",
		};

		auto ret = parse_cmd::parse_pack(ss,vec,"","");
		print_con(ret,1);

	}
}

void test_12()
{
	// "ffffffffffff708bcd7fda620800
	// 450000407a09000080113cc2
	// ac1015c2
	// ac1015ff
	// 202b
	// 202b
	// 002c
	// 834a  // 41 C7 EB 85 41c7eb85
	// 4f4d01c020e00000cd7598012e965cb5dbdb58400177ad6fc50ab91df458a84d57910000";
	// std::string ss = "ffffffffffff708bcd7fda620800450000407a09000080113cc2ac1015c2ac1015ff202b202b002c834aaaee1ffff0800490000000424c03000100000f9090000004c1b8c1b8c1b8c1b89a9deefc";
	std::string ss = "ffffffffffff708bcd7fda620800450000407a09000080113cc2ac1015c2ac1015ff202b202b002c834aaaee1ffff0800490000000424c03000100000f90955000044128000041c7eb859a9deefc";
		std::vector<std::string> vec {
			"(28*)",
			"(24*)",
			"#源IP#",
			"!8![h]",
			"(8*)",
			"#目标IP#",
			"!8![h]",
			"(8*)",
			"#源端口#",
			"!4![u:d:w]",
			"(4*)",
			"#目标端口#",
			"!4![u:d:w]",
			"(4*)",
			"#长度#",
			"!4![u:d:w]",
			"(4*)",
			"(4*)",
			"#接下来是数据包部分#",
			"#检测包头包尾#",
			"{aaee*}",
			"{*eefc}",
			"#查找标记位#",
			"{424c*}",
			"#偏移配置字节#",
			"(14*)",
			"<4>=0",
			"#内容长度#",
			"!4!(4)[u:d:w]",
			"(8*)",
			"#判断数据类型#",
			":2:(0){9000}",
			"#9000类型#",
			"[+4]",
			":5:(0){9550}",
			"#9550类型#",
			"!8![f:w]",
			"!8!(8)[f:w]",
			"!4!(12)[u:d:w]",
			"(20*)"
		};
	auto ret = parse_cmd::parse_pack(ss,vec,"","");
	print_con(ret,1);


}

void test_13()
{
#ifdef WIN32
    system("taskkill /f /t /im DigitalConference.Web.Entry.exe");
#else
    system("killall DigitalConference.Web.Entry");
#endif
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
	// test_3();
	// test_4();
	// test_5();
	// test_6();
	// test_7();
	// test_8();
	// test_9();
	// test_10();
	// test_11();
	test_12();
	// test_13();
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


"符号说明: 字符 * 代表剩余字符左偏,或者右偏, DD 代表数字, SS 代表字符
"命令说明: 跳过命令条件触发时, 会跳过接下来的多条命令, 可用于创建不同分支的格式化效果
"长度类型: d:[4: short 8: int 16: long long] ,f:[8: float 16: double]
"转换操作: [u:d:f:w] [u: 转为无符号] [d: 转为整数-具体类型看长度] [f: 转为浮点数-具体类型看长度] [w: 大小端字节序转换]
"
"命令如下:
"{SS*}"          // 查找指定 SS 字符并截断 ,左偏,
"{*SS}"          // 查找指定 SS 字符并截断 ,右偏
"(DD*)"          // 偏移 DD 个字符并截断 ,左偏"
"(*DD)"          // 偏移 DD 个字符并截断 ,右偏"
"[+D]"           // 格式列表 ,循环切割 DD 个位置 ,直到剩余字符为空
"[+D1+D2]"       // 格式分段列表 ,循环切割 DX 个位置 ,可以由多个 DX 组合,直到剩余字符为空
"<DD>=0"         // 从当前位置标记 DD 个字符 ,放入标记容器的 0 索引
"<D1>(DD)=1"     // 偏移 DD 个位置, 标记 DD 个字符 ,放入标记容器的 1 索引
"<D1>{SS}=2"     // 查找到 SS 字符位置, 标记 DD 个字符 ,放入标记容器的 2 索引
":D1:(D2){SS}"       	// 跳过命令条件 ,D2 为对比的标记容器索引数据 ,SS 为对比数据 ,当条件不满足时跳过 D1 条数据 ,否则继续执行
"!DD![u:d:f:w]"      	// 字节转为数值 ,DD 为切割的长度并将字节转为对应类型 , [] 内选项对应转成无符号类型、大小端转换、浮点数三种选项
"!DD!(D1)[u:d:f:w]"  	// 偏移切割字节转为数值 ,D1 为偏移位置后切割 DD 长度的字节 ,其他相关同上
"!DD!{SS}[u:d:f:w]"  	// 查找切割字节转为数值 ,SS 为查找的偏移子串 ,其他相关同上


符号说明: 字符 * 代表剩余字符左偏,或者右偏, DD 代表数字, SS 代表字符"
命令说明: 跳过命令条件触发时, 会跳过接下来的多条命令, 可用于创建不同分支的格式化效果"
长度类型: d:[4: short 8: int 16: long long] ,f:[8: float 16: double]"
转换操作: [u:d:f:w:h] [u: 转为无符号] [d: 转为整数-具体类型看长度] [f: 转为浮点数] [w: 大小端字节序转换] [h: 转为点十进制IP]"
转换操作: [] 转换操作的无选项是一种特殊情况，该操作会将十六进制字符串退化成二进制数据，可以用于字符串显示"

命令如下:
{SS*}          // 查找指定 SS 字符并截断 ,左偏"
{*SS}          // 查找指定 SS 字符并截断 ,右偏"
(DD*)          // 偏移 DD 个字符并截断 ,左偏"
(*DD)          // 偏移 DD 个字符并截断 ,右偏"
<DD>=0         // 从当前位置标记 DD 个字符 ,放入标记容器的 0 索引"
[+D]           // 格式列表 ,循环切割 DD 个位置 ,直到剩余字符为空"
[+D1+D2]       // 格式分段列表 ,循环切割 DX 个位置 ,可以由多个 DX 组合,直到剩余字符为空"
<D1>(DD)=1     // 偏移 DD 个位置, 标记 DD 个字符 ,放入标记容器的 1 索引"
<D1>{SS}=2     // 查找到 SS 字符位置, 标记 DD 个字符 ,放入标记容器的 2 索引"
<-*>{SS}=3     // 查找到 SS 字符位置, 标记当前位置到 SS 字符最左下标部分 ,放入标记容器的 3 索引"
:D1:(D2){SS}       // 跳过命令条件 ,D2 为对比的标记容器索引数据 ,SS 为对比数据 ,当条件不满足时跳过 D1 条数据 ,否则继续执行"
!DD![u:d:f:w:h]    // 字节转为数值 ,DD 为切割的长度并将字节转为对应类型 , [] 内提供转换选项，如果无选项会退化成字符串"
!DD!(D1)[]         // 偏移切割字节转为数值 ,D1 为偏移位置后切割 DD 长度的字节 ,其他相关同上"
!DD!{SS}[]         // 查找切割字节转为数值 ,SS 为查找的偏移子串 ,其他相关同上"


*/


