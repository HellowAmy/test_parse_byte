# == 函数 ==
# 递归目录并直接包含头文件函数
function(include_directories_sub root_dir)
    if (IS_DIRECTORY ${root_dir})               
        include_directories(${root_dir})
    endif()

    file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*)
    foreach(sub ${ALL_SUB})
        if (IS_DIRECTORY ${root_dir}/${sub})
        include_directories_sub(${root_dir}/${sub})
        endif()
    endforeach()
endfunction()


# 递归获取头文件子目录路径到变量
function(fn_get_include_sub_ls result root_dir)
    set(abs_dir ${CMAKE_SOURCE_DIR}/${root_dir})
    if (IS_DIRECTORY ${abs_dir})
        file(GLOB children RELATIVE ${abs_dir} ${abs_dir}/*)
        set(dirlist "")
        foreach(child ${children})
            if(IS_DIRECTORY ${abs_dir}/${child})
                list(APPEND dirlist ${root_dir}/${child})
                fn_get_include_sub_ls(subdirlist ${root_dir}/${child})
                list(APPEND dirlist ${subdirlist})
            endif()
        endforeach()
        set(${result} ${dirlist} PARENT_SCOPE)
    endif()
endfunction()

# 递归获取目录下的库文件
function(fn_get_lib_ls result root_dir)
    file(GLOB_RECURSE reslib "${root_dir}/*.so*")
    set(${result} ${reslib} PARENT_SCOPE)
endfunction()
# == 函数 ==



# ===== 编译入口 =====
# 项目名
cmake_minimum_required(VERSION 3.22.1)
project(hellow)


# C++ 11 版本
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# 调试模式
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")


# 源文件 : 包含源文件
file(GLOB_RECURSE SRC_WINDOW "window/*.cpp")
file(GLOB_RECURSE SRC_WID "wid/*.cpp")

set(PRO_SRC_FILES 
    main.cpp
    ${SRC_WINDOW} 
    ${SRC_WID} 
)


# 头文件 : 递归扫描子目录头文件
fn_get_include_sub_ls(PRO_HEAD_WID wid)
fn_get_include_sub_ls(PRO_INCLUDE include)

set(PRO_HEAD_DIR
    util
    window
    ${PRO_HEAD_WID}
    ${PRO_INCLUDE}
)


# 库文件 : 指定外部库路径 [需修改库文件与依赖一致]
fn_get_lib_ls(PRO_LIB lib)

set(PRO_LIB_DIY
    ${CMAKE_SOURCE_DIR}/lib/Packet.lib 
    ${CMAKE_SOURCE_DIR}/lib/wpcap.lib 
    ws2_32
)


# # 递归扫描头文件并直接加入
# include_directories_sub(${CMAKE_SOURCE_DIR}/wid)


# 包含头文件
include_directories(${PRO_HEAD_DIR})


# 生成目标
add_executable(${CMAKE_PROJECT_NAME} ${PRO_SRC_FILES})


# 链接库
target_link_libraries(${CMAKE_PROJECT_NAME} ${PRO_QT_MODULE} ${PRO_LIB} ${PRO_LIB_DIY})


