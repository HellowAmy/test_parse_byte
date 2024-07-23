"""
将模版项目复制到新项目路径

argv:
    [1] : 新目录
"""


import sys
import os
import subprocess

# 主函数
if __name__ == "__main__":
    print("== begin ==")
    print("[argv: {}]".format(sys.argv))

    # 判断参数
    if len(sys.argv) < 2:
        print("[argv] : [1: new dir]")
        exit(0)

    # 获取参数
    sys_dir = sys.argv[1]
    new_dir = sys_dir

    # 获取当前目录
    os.chdir("../")
    cur_dir = os.getcwd()

    # 创建新目录
    os.makedirs(new_dir, exist_ok=True)

    if os.path.exists(new_dir):
        # 执行命令-复制
        cmd = "cp -r %s/. %s" % (cur_dir, new_dir)
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

        # 执行命令-删除多余文件
        cmd_rm = "rm -rf %s/.git" % (new_dir)
        subprocess.run(cmd_rm, shell=True, capture_output=True, text=True)

        print("ret: ",result)
        print("cmd: ",cmd)
        print("cur dir: ",cur_dir)
        print("new dir: ",new_dir)
    else:
        print("mkdir faile")

    print("== finish ==")