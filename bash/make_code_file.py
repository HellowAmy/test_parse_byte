"""
创建源文件，需要系统模板文件作为替换

argv:
    [1] : 类名
    [2] : 模版列表下标,template文件夹下模版文件
    [3] : 目标路径,结尾不带斜杠
"""


import sys
import os


# 数组内容替换占位符
def replace_ls(ctx,ls):
    for i in range(len(ls)):
        s = "{%#}"
        s = s.replace("#",str(i+1))
        ctx = ctx.replace(s,ls[i])
    return ctx


# 通过分隔符获取模板的头文件和源文件内容
def get_file_ctx(path):
    with open(path,"r") as file:
        file_txt = file.read()
        ls = file_txt.split("#==========#")
        return ls[0],ls[1]  


# 写入目标文件
def write_file(ctx,path):
    with open(path, 'w') as file:
        file.write(ctx)
        print("[write : {}]".format(path))


# 替换占位符-头文件
def perse_h(ctx,class_name):
    return replace_ls(ctx,[class_name,class_name.upper()])


# 替换占位符-源文件
def perse_c(ctx,class_name):
    return replace_ls(ctx,[class_name])


# 获取模板列表
def get_template_ls(path):
    abs_path = os.path.abspath(path)
    file_list = []
    for file_name in os.listdir(abs_path):
        abs_file = abs_path + "/" + file_name
        if os.path.isfile(abs_file):
            file_list.append(abs_file)
    return file_list


# 显示模版列表
def show_template_ls():
    print("== template ls ==")
    ls = get_template_ls("./template/")
    for i in range(len(ls)):
        s = "[{}] [{}]".format(i,ls[i])
        print(s)


# 主函数
if __name__ == "__main__":
    print("== begin ==")
    print("[argv: {}]".format(sys.argv))

    # 判断参数
    if len(sys.argv) < 4:
        print("[argv] : [1: class name] [2: template index] [3: create path]")
        show_template_ls()
        exit(0)

    # 获取参数
    sys_name = sys.argv[1]
    sys_template = sys.argv[2]
    sys_path = sys.argv[3]


    
    # 解析参数
    template_path = "template/"
    ls_template = get_template_ls(template_path)
    s_name = sys_name
    s_template = ls_template[int(sys_template)]
    s_path = sys_path
    path_h = sys_path + "/" + s_name + ".h"
    path_c = sys_path + "/" + s_name + ".cpp"

    # 获取模板文件
    tup = get_file_ctx(s_template)
    template_h ,template_c = tup

    print("== finish ==:",len(template_h),len(template_c))

    # 写入到目标
    if len(template_h) > 0:
        write_file(perse_h(template_h,s_name),path_h)
    if len(template_c) > 0:  
        write_file(perse_c(template_c,s_name),path_c)

    print("== finish ==")