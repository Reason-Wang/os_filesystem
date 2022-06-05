//
// Created by WRX on 2022/5/27.
//
#include "file.h"

FILE * file;

struct SuperBlock super_block = {
        .inode_block_num=INODE_BLOCK_NUM,
        .data_block_num=BLOCK_NUM-INODE_BLOCK_NUM-1,
        .s_free={0},
        .root_dir_address=0,
        .users_num=1,
};

char * disk_path = "..\\disk\\DISK";

char * cur_user_name = "root";

struct User cur_user;

struct DirItem cur_dir;



// 块号转化为地址
int convert_block_num_to_address(unsigned short block_num){
    return block_num*BLOCK_SIZE;
}

unsigned short convert_address_to_block_num(int block_address){
    return block_address/BLOCK_SIZE;
}


void print_superblock(){
    printf("superblock: inode block number: %d, data block num: %d\n", super_block.inode_block_num, super_block.data_block_num);
    printf("superblock: sfree: ");
    for(int i = 0; i <= super_block.s_free[0]; i++){
        printf(" [%d]:%d ", i, super_block.s_free[i]);
    }
    printf("\n");
}

void print_grouper(){
    fseek(file, 0, SEEK_SET);
    unsigned short block_num = super_block.s_free[1];
    struct Grouper grouper;
    int address = convert_block_num_to_address(block_num);
    while(true){
        fseek(file, address, SEEK_SET);
        fread(&grouper, sizeof(struct Grouper), 1, file);
        printf("group block: %d ", block_num);
        for(int i = 0; i <= grouper.s_free[0]; i++){
            printf(" [%d]:%d ", i, grouper.s_free[i]);
        }
        printf("\n");

        block_num = grouper.s_free[1];
        address = convert_block_num_to_address(block_num);
        if(block_num == 0)
            break;
    }
}

void print_users(){
    fseek(file, USER_ADDRESS_START, SEEK_SET);
    struct User users[MAX_USER_NUM];
    fread(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);
    for(int i=0; i< super_block.users_num; i++){
        if(strcmp(users[i].name, "") != 0){
            printf("user:%s password:%s group:%c,", users[i].name, users[i].password, users[i].group);
        }
    }
    printf("\n");
}


/*
 * 初始化工作，若存在DISK文件，
 * 则从DISK进行初始化，否则新建文件并格式化
 */
void init(){
//    FILE * file;
    if((file=fopen(disk_path, "rb")) == NULL){
//        fd = open(disk_path, O_CREAT);
        fprintf(stderr, "No disk file found, create new.\n");
        format();
//        print_superblock(super_block);

        return;
    }
    file = fopen(disk_path, "rb+");     // 文件必须存在，读写文件

    // 初始化超级块
    printf("Read superblock\n");
    fseek(file, 0, SEEK_SET);
    fread(&super_block, sizeof(struct SuperBlock), 1, file);
    printf("Done!\n");

    // 设置当前目录
    int root_dir_address = super_block.root_dir_address;
    fseek(file, root_dir_address, SEEK_SET);
    fread(&cur_dir, sizeof(struct DirItem), 1, file);
//    printf("cur_dir: %s\n", cur_dir.item_name);

//    print_superblock();
//    print_users();

}


/*
 * 格式化磁盘，划分空间
 */
void format(){
    // 打开磁盘块文件，若无则新建一个
    if((file = fopen(disk_path, "wb+")) == NULL) {                     // 读写，若不存在则新建
        fprintf(stderr, "Create file error.");
        exit(-1);
    }
    fseek(file, BLOCK_SIZE*BLOCK_NUM, SEEK_SET);
    fputc('\0', file);


    // 格式化超级块
    super_block.users_num = 3;                                         // 包含root用户和测试用户
    super_block.s_free[0] = EMPTY_BLOCK_NUM_PER_GROUP;
    super_block.s_free[1] = EMPTY_BLOCK_NUM_PER_GROUP+INODE_BLOCK_NUM+INODE_BLOCK_NO_START-1;
    for(int i = 2; i <= EMPTY_BLOCK_NUM_PER_GROUP; i++){                                           // s_free[0] 50
        super_block.s_free[i] = EMPTY_BLOCK_NUM_PER_GROUP+INODE_BLOCK_NUM+INODE_BLOCK_NO_START-i;  //       [1] 71
    }                                                                                              //       [2] 70
                                                                                                   //       ...
                                                                                                   //       [50]22
    // 对每个组长块进行格式化
    fseek(file, INODE_ADDRESS_START, SEEK_SET);
    for(int i = EMPTY_BLOCK_NUM_PER_GROUP+INODE_BLOCK_NUM+INODE_BLOCK_NO_START-1; i < BLOCK_NUM; i+=EMPTY_BLOCK_NUM_PER_GROUP){
        fseek(file, i*BLOCK_SIZE, SEEK_SET);                           // 找到对应块位置
        // 剩余组空闲块数量
        int number_of_empty = EMPTY_BLOCK_NUM_PER_GROUP < BLOCK_NUM-i ? EMPTY_BLOCK_NUM_PER_GROUP : BLOCK_NUM-i;
        struct Grouper grouper;
        grouper.s_free[0] = number_of_empty;                           // 第一个元素为空闲块数量
        if(EMPTY_BLOCK_NUM_PER_GROUP + i < BLOCK_NUM)
            grouper.s_free[1] = EMPTY_BLOCK_NUM_PER_GROUP + i;         // 第二个元素为下一空闲块编号
        else
            grouper.s_free[1] = 0;

        for(int j = 2; j <= number_of_empty; j++){
            grouper.s_free[j] = number_of_empty + i + 1 - j;
        }
        fwrite(&grouper, sizeof(struct Grouper), 1, file);
    }

//    print_grouper();


    // 创建根目录
    int inode_address = ialloc();
    int block_address = balloc();
    super_block.root_dir_address = block_address;

    struct DirItem dir_list[MAX_DIR_NUM];
    for(int i = 0; i < MAX_DIR_NUM; i++){
        strcpy(dir_list[i].item_name, "");
        dir_list[i].inode_address=0;
    }

    strcpy(dir_list[0].item_name, "~");
    dir_list[0].inode_address = inode_address;

    struct Inode inode = {
            .group='R',                              // 根目录为特殊组R
            .link_num=1,
            .update_time=time(NULL),
            .size=BLOCK_SIZE,
            .block_address={-1},
            .type=DIRECTORY,
            .mode=0b001001001                       //  所有组可读但不可修改根目录
    };
    strcpy(inode.owner_name, "root");
    inode.block_address[0] = block_address;

    fseek(file, block_address, SEEK_SET);           // 写目录到磁盘块
    fwrite(dir_list, sizeof(dir_list), 1, file);
    fseek(file, inode_address, SEEK_SET);
    fwrite(&inode, sizeof(struct Inode), 1, file);  // 写索引节点


    cur_dir = dir_list[0];                          // 当前目录为根目录'.'

    fseek(file, 0, SEEK_SET);
    fwrite(&super_block, sizeof(struct SuperBlock), 1, file);
    fprintf(stderr, "Write super block to disk.\n");

    fseek(file, 0, SEEK_SET);
    fread(&super_block, sizeof(struct SuperBlock), 1, file);
//    print_superblock();


    // 创建root用户和测试用户
    struct User users[MAX_USER_NUM] = {{.group=GROUP_A, .name="", .password=""}};
    strcpy(users[0].name, "root");
    strcpy(users[0].password, "root");
    users[0].group = 'R';
    strcpy(users[1].name, "usera");
    strcpy(users[1].password, "usera");
    users[1].group = GROUP_A;
    strcpy(users[2].name, "userb");
    strcpy(users[2].password, "userb");
    users[2].group = GROUP_B;


    fseek(file, USER_ADDRESS_START, SEEK_SET);
    fwrite(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);
//    print_users();

}


/*`
 * 分配i节点
 * 只传送空闲i节点地址，不对i节点进行其他操作
 * 成功返回i节点地址，失败返回-1
 */
int ialloc(){
    struct Inode inode;
    int i;
    for(i = INODE_ADDRESS_START; i < INODE_ADDRESS_END; i += INODE_SIZE){
        fseek(file, i, SEEK_SET);
        fread(&inode, sizeof(struct Inode), 1, file);
        if(inode.link_num == 0)
            break;
    }
    if(i >= INODE_ADDRESS_END)
        return -1;
    return i;
}

/*
 * 释放i节点
 * 成功返回0，失败返回-1
 */
int ifree(int inode_address){
    if((inode_address - INODE_ADDRESS_START)%INODE_SIZE != 0){
        fprintf(stderr, "ifree: inode address error.");
        return -1;
    }
    struct Inode inode;
    fseek(file, inode_address, SEEK_SET);
    fread(&inode, sizeof(struct Inode), 1, file);
    if(inode.link_num == 0){
        fprintf(stderr, "ifree: inode has not been used.");
        return -1;
    }
    inode.link_num = 0;         //设置为空闲块
    fseek(file, inode_address, SEEK_SET);
    fwrite(&inode, sizeof(struct Inode), 1, file);
    return 0;
}


/*
 * 按成组链接法分配磁盘块
 * 成功返回块地址，失败返回-1
 */
int balloc(){
    if(super_block.s_free[0] == 1){
        if(super_block.s_free[1] == 0){
            fprintf(stderr, "balloc: no empty blcok error.");
            return -1;
        }
        else{                                       // 只有一个空闲块
            int N = super_block.s_free[1];          // 该块为组长块
            int address = convert_block_num_to_address(N);
            fseek(file, address, SEEK_SET);
            struct Grouper grouper;
            fread(&grouper, sizeof(struct Grouper), 1, file);
            // 复制组长块中栈到超级块
            memcpy(super_block.s_free, grouper.s_free, sizeof(unsigned short)*(EMPTY_BLOCK_NUM_PER_GROUP+1));
            return address;                         // 该块已经不为组长块，且一定为空闲块

        }
    }
    else{
        int block_num = super_block.s_free[super_block.s_free[0]];
        super_block.s_free[0]--;
        return convert_block_num_to_address(block_num);
    }
}


/*
 * 按成组链接法回收磁盘块
 */
int bfree(int block_address){
    if(super_block.s_free[0] == EMPTY_BLOCK_NUM_PER_GROUP){
        struct Grouper grouper;
        memcpy(grouper.s_free, super_block.s_free, sizeof(unsigned short)*(EMPTY_BLOCK_NUM_PER_GROUP+1));
        fseek(file, block_address, SEEK_SET);
        fwrite(&grouper, sizeof(struct Grouper), 1, file);
        super_block.s_free[0] = 1;
        super_block.s_free[1] = convert_address_to_block_num(block_address);
    }
    else{
        super_block.s_free[0]++;
        super_block.s_free[super_block.s_free[0]] = convert_address_to_block_num(block_address);
    }

//    print_superblock();

}

// 检查用户名和密码是否正确
// 成功返回用户索引，失败返回-1
int check_user(struct User * all_users, char *user_name, char *password){

    for(int i = 0; i < MAX_USER_NUM; i++){
        if(strcmp(all_users[i].name, user_name) == 0){
            if(strcmp(all_users[i].password, password) == 0)
                return i;
            else
                return -1;
        }
    }
    return -1;
}

/*
 * 用户登录
 */
void login(){
    fseek(file, USER_ADDRESS_START, SEEK_SET);
    struct User all_users[MAX_USER_NUM];
    fread(all_users, sizeof(struct User)*MAX_USER_NUM, 1, file);

    char user_name[15];
    char password[15];

    printf("Login\n");
    printf("user name: ");
    gets(user_name);
    printf("password: ");
    gets(password);
    int i;
    while((i=check_user(all_users, user_name, password)) < 0){
        printf("User name or password incorrect, try again!\n");
        printf("user name: ");
        gets(user_name);
        printf("password: ");
        gets(password);
    }
    strcpy(cur_user.name, user_name);
    strcpy(cur_user.password, password);
    cur_user.group = all_users[i].group;
    printf("Login successfully!\n");

}

/*
 * 检查写权限
 * 若包含写权限则返回true，否则返回false
 */
bool check_write_permission(struct Inode inode){
    int filemode;
    if(strcmp(cur_user.name, inode.owner_name)==0)
        filemode = OWNER_MODE;
    else if(cur_user.group == inode.group)
        filemode = GROUP_MODE;
    else
        filemode = OTHER_MODE;
    if(((inode.mode >> filemode>>WRITE_SHIFT)&1)==0 && (strcmp(cur_user.name, "root") != 0))
    {
        fprintf(stderr, "permission denied.\n");
        return false;
    }
    return true;
}
