//
// Created by WRX on 2022/5/27.
//

#ifndef UNTITLED_FILE_H
#define UNTITLED_FILE_H

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

#define BLOCK_SIZE 512                     // 磁盘块大小512B
#define INODE_SIZE 64                      // i节点大小64B
#define EMPTY_BLOCK_NUM_PER_GROUP 50       // 每组空磁盘块最多数量
#define INODE_BLOCK_NUM 20                 // i节点块数
#define BLOCK_NUM 200                      // 总磁盘块数

#define INODE_BLOCK_NO_START 2             // 节点区起始块号
#define INODE_ADDRESS_START (BLOCK_SIZE*INODE_BLOCK_NO_START)
                                           // 节点区起始地址
#define INODE_ADDRESS_END (INODE_ADDRESS_START+INODE_BLOCK_NUM*BLOCK_SIZE)
                                           // 节点区终点地址

#define MAX_DIR_NAME_SIZE 14               // 目录名称长度
#define MAX_DIR_NUM 16                     // 每个目录内最多的文件或目录数，要
                                           // 求所有目录在一个磁盘块内，即该项
                                           // * sizeof(DirItem) < BLOCK_SIZE
#define NORMAL 'n'                         // 正规文件
#define DIRECTORY 'd'                      // 目录文件

#define USER_ADDRESS_START (BLOCK_SIZE)    // 用户区起始地址
#define MAX_USER_NUM  16                   // 用户最多数量

#define READ_SHIFT 0                       // 读权限偏移量
#define WRITE_SHIFT 1                      // 写权限偏移量
#define EXECUTE_SHIFT 2                    // 执行权限偏移量

#define OWNER_MODE 6                       // 文件拥有者
#define GROUP_MODE 3                       // 拥有者同组
#define OTHER_MODE 0                       // 其他情况

#define MAX_NAME_SIZE 28                   // 临时定义
#define GROUP_A 'A'                        // A用户组
#define GROUP_B 'B'                        // B用户组
#define GROUP_C 'C'                        // C用户组

struct SuperBlock{
    unsigned short inode_block_num;        // i节点磁盘块数量
    unsigned short data_block_num;         // 数据块数量

    unsigned short s_free[EMPTY_BLOCK_NUM_PER_GROUP+1];
                                           // 空闲块栈

    int root_dir_address;                  // 根目录地址
    unsigned short users_num;              // 有效用户数量


};

// 组长块，仅包括空闲栈
struct Grouper{
    unsigned short s_free[EMPTY_BLOCK_NUM_PER_GROUP+1];
};

struct Inode{
//    unsigned short i_no;                 // 文件磁盘块号 0-65535

    char owner_name[10];                   // 文件所属用户
    char group;                            // 文件所属用户组
    char type;                             // 文件类型 正规文件，目录文件
    unsigned short mode;                   // 文件存取权限 0000000xwrxwrxwr

    unsigned int size;                     // 文件大小 0-4294967294 Bytes
    unsigned short link_num;               // 文件链接计数
    time_t update_time;                    // 文件上次更新时间

    int block_address[8];                  // 直接地址
//    unsigned short groupers[BLOCK_NUM/EMPTY_BLOCK_NUM_PER_GROUP];
                                           // 空闲块编号
};


/*
 * 用户
 * 包含用户名，用户组，用户密码
 */
struct User{
    char name[15];
    char password[16];
    char group;
};

/*
 * 文件或目录名
 * 16字节，每个目录紧凑排列在磁盘块上
 */
struct DirItem{
    char item_name[MAX_DIR_NAME_SIZE]; // 文件或目录名
    int inode_address;                 // 文件或目录对应i节点地址
};

/*
 * 初始化工作，若存在DISK文件，
 * 则从DISK进行初始化，否则新建文件并格式化
 */
void init();

/*
 * 格式化磁盘，划分空间
 */
void format();

/*
 * 命令行工具
 */
void sh();

/*
 * 分配i节点
 * 只传送空闲i节点地址，不对i节点进行其他操作
 * 成功返回i节点地址，失败返回-1
 */
int ialloc();

/*
 * 释放i节点
 * 成功返回0，失败返回-1
 */
int ifree(int inode_address);


/*
 * 按成组链接法分配磁盘块
 * 成功返回块地址，失败返回-1
 */
int balloc();


/*
 * 按成组链接法回收磁盘块
 */
int bfree(int block_address);

/*
 * 用户登录
 */
void login();

/*
 * 检查写权限
 * 若包含写权限则返回true，否则返回false
 */
bool check_write_permission(struct Inode inode);

// 块号转化为地址
int convert_block_num_to_address(unsigned short block_num);

// 地址转化为块号
unsigned short convert_address_to_block_num(int block_address);

#endif //UNTITLED_FILE_H
