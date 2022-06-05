//
// Created by WRX on 2022/5/28.
//

#ifndef FILESYSTEM_SH_H
#define FILESYSTEM_SH_H

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_PREV_COMMANDS 16      //最多记录的命令数量
#define MAX_ARG_NUM 4             //每个命令参数的数量
#define MAX_ARG_LENGTH 4096       // 每个参数的最大长度

struct Cmd{
    char argv[MAX_ARG_NUM][MAX_ARG_LENGTH]; //命令，第一个为命令名称
    unsigned short length;        // 有效长度
};


/*
 * 命令行
 */
void sh();

/*
 * ls命令，用于浏览文件
 */
void ls();

/*
 * 跳转到目录
 */
bool Cd(char skip_address[]);

/*
 *  创建目录
 */
void Mkdir(char create_dir_name[]);

/*
 * 删除目录
 */
void Rmdir(char delete_dir_name[]);

/*
 * 退出
 */
void Exit();

/*
 * 创建用户
 */
void useradd();

/*
 * 删除用户，格式为
 * userdel user_name
 */
void userdel();


void touch(int parinoAddr,char name[],char buf[]);

bool create(int parinoAddr,char name[],char buf[]);

bool del(int parinoAddr,char name[]);

void cls();

void wi(int parinoAddr,char name[],char buf[]);

/*
 * 输出文件内容
 */
void cat(char filename[]);

/*
 * 输出超级块信息
 */
void super();

/*
 * 帮助函数
 */
void help();

/*
 * 输出用户信息
 */
void listuser();

/*
 * 修改文件权限
 */
void change_permission(char filename[],char permission[]);

/*
 * 格式化磁盘
 */
bool formattor();

#endif //FILESYSTEM_SH_H
