//
// Created by WRX on 2022/5/28.
//

#include "sh.h"
#include "file.h"

// 用于保持已有的命令
//char commands[MAX_PREV_COMMANDS][MAX_ARG_LENGTH*MAX_ARG_NUM] = {"\0"};

//char cur_command = "";                // 指向当前命令文本

struct Cmd cur_cmd = {.argv={{'\0'}}, .length=0}; // 当前指令结构体

char command_buf[MAX_ARG_LENGTH*MAX_ARG_NUM] = {'\0'};    // 当前命令缓冲区

char * buf_ptr = command_buf;                             // 当前输入指针

extern struct SuperBlock super_block;             // 超级块
extern FILE * file;                               // 磁盘文件
extern struct DirItem cur_dir;                    // 当前目录
//extern char * cur_user_name;                      // 当前用户名
extern struct User cur_user;                      // 当前用户

int path_position=1;                              // 路径名最后一个元素的位置
char cur_path_name[200] = {"~"};                  // 完整路径名
//char cur_dir_name[40] = {"~"};                    // 当前目录名

char buf[4096] = {""};


// 打印命令
void print_cmd(){
    printf("<");
    for(int i = 0; i < cur_cmd.length; i++){
        printf("%s ", cur_cmd.argv[i]);
    }
    printf(">\n");
}


/*
 * 将文本命令按照空格划分并写入Cmd中
 */
void parse_cmd(char * command, struct Cmd * cmd){
    int i = 0;
    cmd->length = 0;
    const char delimeter[2] = " ";
    char * pch = strtok(command, delimeter);
    while(pch != NULL){
//        printf("%s", pch);
        if(i < MAX_ARG_NUM)
        {
            strcpy(cmd->argv[i++], pch);
            cmd->length += 1;
        }
        pch = strtok(NULL, delimeter);
    }
}


/*
 * 按照字符获取输入，对字符进行判断以处理相应命令
 */
int getcmd(){
    strcpy(buf, "\0");
    gets(buf);
    parse_cmd(buf, &cur_cmd);

    return 0;

}


/*
 * 命令行
 */
void sh(){
    printf("\n[%s@%s]>", cur_user.name, cur_path_name);
    while(getcmd() >= 0){
//        print_cmd();
        if(strcmp(cur_cmd.argv[0], "ls") == 0){
            ls();
        }
        else if(strcmp(cur_cmd.argv[0], "exit") == 0){
            Exit();
        }
        else if(strcmp(cur_cmd.argv[0],"cd") == 0){
            bool flag=Cd(cur_cmd.argv[1]);
        }
        else if(strcmp(cur_cmd.argv[0],"mkdir") == 0){
            Mkdir(cur_cmd.argv[1]);
        }
        else if(strcmp(cur_cmd.argv[0],"rmdir")==0){
            Rmdir(cur_cmd.argv[1]);
        }
        else if(strcmp(cur_cmd.argv[0], "useradd") == 0){
            useradd();
        }
        else if(strcmp(cur_cmd.argv[0], "userdel") == 0){
            userdel();
        }
        else if(strcmp(cur_cmd.argv[0], "touch") == 0){
            touch(cur_dir.inode_address,cur_cmd.argv[1],cur_cmd.argv[2]);
        }
        else if(strcmp(cur_cmd.argv[0], "rm") == 0){
            del(cur_dir.inode_address, cur_cmd.argv[1]);
        }
        else if(strcmp(cur_cmd.argv[0], "cls") == 0){
            cls();
        }
        else if(strcmp(cur_cmd.argv[0], "write") == 0){
            wi(cur_dir.inode_address,cur_cmd.argv[1],cur_cmd.argv[2]);
        }
        else if(strcmp(cur_cmd.argv[0], "cat") == 0){
            cat(cur_cmd.argv[1]);
        }
        else if(strcmp(cur_cmd.argv[0], "logout")== 0){
            cls();
            break;
        }
        else if(strcmp(cur_cmd.argv[0], "super")==0){
            super();
        }
        else if(strcmp(cur_cmd.argv[0], "help") == 0){
            help();
        }
        else if(strcmp(cur_cmd.argv[0], "listuser") == 0){
            listuser();
        }
        else if(strcmp(cur_cmd.argv[0], "chmod") == 0){
            change_permission(cur_cmd.argv[1], cur_cmd.argv[2]);
        }
        else if(strcmp(cur_cmd.argv[0], "format") == 0){
            if(formattor())
                break;
        }
        else{
            fprintf(stderr, "no such command: ");
            print_cmd();
        }
        printf("\n[%s@%s]>", cur_user.name, cur_path_name);
    }
}


/*
 * ls命令，用于浏览文件
 */
void ls(){
    int inode_address = cur_dir.inode_address;
    struct Inode inode;
    fseek(file, inode_address, SEEK_SET);
    fread(&inode, sizeof(struct Inode), 1, file);
    int block_address = inode.block_address[0];
    struct DirItem dir_list[MAX_DIR_NUM];
    fseek(file, block_address, SEEK_SET);
    fread(&dir_list, sizeof(dir_list), 1, file);

    if(cur_cmd.length == 1) {
        for (int i = 0; i < MAX_DIR_NUM; i++) {
            if (strcmp(dir_list[i].item_name, "") != 0) {
                printf("%s  ", dir_list[i].item_name);
            }
        }
        printf("\n");
    }
    else if(cur_cmd.length == 2 && strcmp(cur_cmd.argv[1], "-l")==0){
        char property[11] = {'-'};
        property[10] = '\0';
//        printf("property: %s", property);
        char owner_name[15];
        char group_name;
        unsigned int size;
        struct tm * ptr;
        struct Inode inode;
        for(int i = 0; i < MAX_USER_NUM; i++){
            if(strcmp(dir_list[i].item_name, "") != 0){
                fseek(file, dir_list[i].inode_address, SEEK_SET);
                fread(&inode, sizeof(struct Inode), 1, file);
                if(inode.type == NORMAL)
                    property[0] = '-';
                else
                    property[0] = 'd';
                int t = 8;
                while(t >= 0){
                    property[9-t] = '-';
                    if(((inode.mode>>t)&1)==1){
                        if(t%3==2)
                            property[9-t] = 'x';
                        if(t%3==1)
                            property[9-t] = 'w';
                        if(t%3==0)
                            property[9-t] = 'r';
                    }
                    t--;
                }
                strcpy(owner_name, inode.owner_name);
                group_name = inode.group;
                size = inode.size;
                ptr = gmtime(&inode.update_time);

                printf("%s\t%s\t%c\t%dB\t%d.%d.%d %02d:%02d:%02d  ", property,owner_name,group_name, size, 1900+ptr->tm_year,ptr->tm_mon,ptr->tm_mday, (8+ptr->tm_hour)%24,ptr->tm_min,ptr->tm_sec);
                printf("%s\n", dir_list[i].item_name);
            }
        }
    }
}



bool Cd(char skip_address[])    //目前是单级跳转,这是查询不是建立，不会对目录进行任何修改
{
    //由cur_dir查找当前的目录
    struct Inode temp;
    fseek(file, cur_dir.inode_address, SEEK_SET);
    fread(&temp, sizeof(struct Inode), 1, file);
    fseek(file, temp.block_address[0], SEEK_SET);
    int nothing = 0;
    int root_root = 0;
    struct DirItem temp_dir[MAX_DIR_NUM];   //这里面装的是当前目录连接的最多16个文件或者目录,第一个是自己，第二个是父亲目录
    fread(&temp_dir, sizeof(temp_dir), 1, file);   //连读16个struct DirItem
    int flag = 0;
    for (int i = 0; i < MAX_DIR_NUM; i++) {
        if (strcmp(temp_dir[i].item_name, skip_address) == 0) {  //如果想跳的文件找到了

            if (strcmp(skip_address, "..") == 0) {           //如果想跳的命令刚好是上一级目录

                //if(strcmp(cur_dir.item_name,"~")==0){       //如果刚好是根目录想跳上一级目录
                //printf("this directory is root, its father directory is not exist!\n");
                //root_root=1;
                //}
                //if (!root_root) {           //如果刚好是非根目录想跳上一级目录
                while (cur_path_name[path_position - 1] != '\\') {
                    cur_path_name[path_position - 1] = '\0';
                    path_position--;
                }
                cur_path_name[path_position - 1] = '\0';
                path_position--;
                //}
            } else if (strcmp(skip_address, ".") == 0) { nothing = 1; }      //如果想跳的是目录是自己,就无事情发生
            else {          //如果想跳的是下一级目录
                cur_path_name[path_position++] = '\\';
                int j = 0;
                while (temp_dir[i].item_name[j] != '\0') {
                    cur_path_name[path_position++] = temp_dir[i].item_name[j];
                    j++;
                }
                //printf("cur_path_name:%s", cur_path_name);
            }
            if (!nothing) {
                cur_dir.inode_address = temp_dir[i].inode_address;
            }
            flag = 1;
            //printf("\ncurrent directory  i_node address:%d\n",cur_dir.inode_address);
            break;
        }
    }

    if (flag == 0) {
        printf("cd:this directory is not exist!\n");
        return false;
    }
    return true;
}



void Mkdir(char create_dir_name[])      //这里建立的时候不能忘记父亲目录名,默认已经在当前目录下
{
    if(cur_cmd.length != 2){
        fprintf(stderr, "mkdir: usage: mkdir dir_name\n");
        return;
    }
    struct Inode temp1;        //temp1用于当前目录的i结点地址
    fseek(file,cur_dir.inode_address,SEEK_SET);
    fread(&temp1,sizeof (struct Inode),1,file);
    fseek(file,temp1.block_address[0],SEEK_SET);
    struct DirItem temp_dir[MAX_DIR_NUM];   //这里面装的是当前目前连接的最多16个文件或者目录,第一个是自己，第二个是父目录
    fread(&temp_dir,sizeof (temp_dir),1,file);
    int i=0;
    int sign=1;
    while (strcmp(temp_dir[i].item_name,"")!=0){
        //printf("item_name:%s\n",temp_dir[i].item_name);
        if(strcmp(temp_dir[i].item_name,create_dir_name)==0){
            sign=0;
            break;
        }
        i++;
    }
    if(sign) {
        int length = i;
        if (length == MAX_DIR_NUM) {
            fprintf(stderr, "mkdir: Exceeds max number of files of directories!\n");
        } else {
//            printf("create dir!\n");
            strcpy(temp_dir[length].item_name, create_dir_name);
            temp_dir[length].inode_address = ialloc();
//            printf("current temp_dir[%d] get inode_address:%d\n", length, temp_dir[length].inode_address);
//            printf("get inode_address:%d ", temp_dir[length].inode_address);
            fseek(file, temp_dir[length].inode_address, SEEK_SET);
            struct Inode temp2;         //temp2用于保存新建立目录对应的i结点地址
            fread(&temp2, sizeof(struct Inode), 1, file);
            //把这个全新的i结点读出来,读到temp2里
            temp2.block_address[0] = balloc();     //目录i结点8个块只用1个块,只用分配1个块,这个块放的是struct DirItem型数组
            temp2.type = 'd';
            temp2.link_num=1;
            temp2.mode=0b111111001;
            strcpy(temp2.owner_name,cur_user.name);
            temp2.group=cur_user.group;
            temp2.update_time=time(NULL);
//            printf("current inode address:%d get block_address:%d\n",temp_dir[length].inode_address, temp2.block_address[0]);
//            printf(" get block_address:%d\n", temp2.block_address[0]);
            //注:这里i结点初始化只初始化了物理块的地址,其他的全没写!
            struct DirItem new_dir_matrix[MAX_DIR_NUM];
            for (int i = 0; i < MAX_DIR_NUM; i++) {
                strcpy(new_dir_matrix[i].item_name, "");
                new_dir_matrix[i].inode_address = 0;
            }
            strcpy(new_dir_matrix[0].item_name, ".");
            new_dir_matrix[0].inode_address = temp_dir[length].inode_address;
//            printf("\ncurrent directory %s address:%d\n",create_dir_name,new_dir_matrix[0].inode_address);
            strcpy(new_dir_matrix[1].item_name, "..");
            new_dir_matrix[1].inode_address = cur_dir.inode_address;
//            printf("current father directory %s address:%d\n",cur_dir.item_name,cur_dir.inode_address);
            fseek(file, temp_dir[length].inode_address, SEEK_SET);
            fwrite(&temp2, sizeof(struct Inode), 1, file);
            fseek(file, temp2.block_address[0], SEEK_SET);
            fwrite(&new_dir_matrix, sizeof(new_dir_matrix), 1, file);
            fseek(file, temp1.block_address[0], SEEK_SET);
            fwrite(&temp_dir, sizeof(temp_dir), 1, file);
        }
    }
    else{
        printf("mkdir: this directory is already existed!\n");
    }
}




void Rmdir(char delete_dir_name[])	//目录删除函数,默认已经在当前目录下,删除的是当前目录里面的一个目录
{
    struct DirItem saving1=cur_dir;
    bool judge=Cd(delete_dir_name);    //跳转的目录cur_dir就是要删除的目录
    struct DirItem saving2=cur_dir;
    if(judge) {         //下面是要把我要删除的块读出来
        struct Inode temp1;         //temp1保存的是当前目录也就是要删除目录的i结点
        fseek(file, cur_dir.inode_address, SEEK_SET);
        fread(&temp1, sizeof(struct Inode), 1, file);
        if( check_write_permission(temp1)) {
            fseek(file, temp1.block_address[0], SEEK_SET);
            struct DirItem temp_dir1[MAX_DIR_NUM];   //这里面装的是当前目前连接的最多16个文件或者目录,第一个是自己，第二个是父目录
            fread(&temp_dir1, sizeof(temp_dir1), 1, file);   //连读16个struct DirItem
            int i = 0;
            while (strcmp(temp_dir1[i].item_name, "") != 0) {
                i++;
            }
            int length = i;
            for (int i = 2; i < length; i++) {    //如果当前目录里面已经什么都没有了,这个循环就直接跳过
                struct Inode temp2;
                fseek(file, temp_dir1[i].inode_address, SEEK_SET);
                fread(&temp2, sizeof(struct Inode), 1, file);
                if (temp2.type == 'd') {    //如果当前目录里面还有目录接着跳
                    Rmdir(temp_dir1[i].item_name);
                    cur_dir = saving2;
                } else {  //如果当前目录里面有文件,直接删除
                    //执行文件删除函数,过程如下：
                    //通过文件的i结点找到它对应的物理快地址，释放那些物理块们
                    //之后释放掉i结点
                    del(cur_dir.inode_address, temp_dir1[i].item_name);
                }
            }

            struct Inode temp3;
            fseek(file, cur_dir.inode_address, SEEK_SET);
            fread(&temp3, sizeof(struct Inode), 1, file);
            bfree(temp3.block_address[0]);
            ifree(cur_dir.inode_address);

            struct Inode temp4;
            fseek(file, saving1.inode_address, SEEK_SET);
            fread(&temp4, sizeof(struct Inode), 1, file);
            struct DirItem temp_dir2[MAX_DIR_NUM];
            fseek(file, temp4.block_address[0], SEEK_SET);
            fread(&temp_dir2, sizeof(temp_dir2), 1, file);
            for (int i = 1; i < MAX_DIR_NUM; i++) {
                if (strcmp(temp_dir2[i].item_name, delete_dir_name) == 0) {
                    strcpy(temp_dir2[i].item_name, "");
                    temp_dir2[i].inode_address = 0;
                    break;
                }
            }
            fseek(file, temp4.block_address[0], SEEK_SET);
            fwrite(&temp_dir2, sizeof(temp_dir2), 1, file);
            cur_dir = saving1;

            while(cur_path_name[path_position-1]!='\\'){
                cur_path_name[path_position-1]='\0';
                path_position--;
            }
            cur_path_name[path_position-1]='\0';
            path_position--;
        }
        else{
            printf("current user don't own this permission!");
            cur_dir=saving1;
        }
    }
    else{
        printf("rmdir: directory not exist!");
    }
}


void Exit()
{
    fseek(file,0,SEEK_SET);
    fwrite(&super_block,sizeof(super_block),1,file);
    printf("successfully exit!");
    exit(0);
}


/*
 * 创建用户，格式为：
 * useradd user_name password group
 */
void useradd(){
    // root用户才可添加
    if(strcmp(cur_user.name, "root") != 0){
        fprintf(stderr, "useradd: only root can add user.\n");
        return;
    }

    // 检查useradd格式
    if(cur_cmd.length != 4){
        fprintf(stderr, "useradd: useradd user_name password group.\n");
        return;
    }

    fseek(file, USER_ADDRESS_START, SEEK_SET);
    struct User users[MAX_USER_NUM];
    fread(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);

    // 检查是否存在同名用户
    for(int i = 0; i < super_block.users_num; i++){
        if(strcmp(users[i].name, cur_cmd.argv[1]) == 0){
            fprintf(stderr, "useradd: user already exists.\n");
            return;
        }
    }

    // 检查是组号是否合法
    char group = cur_cmd.argv[3][0];
//    printf("group: %c\n", group);
    if((isalpha(group))
    && (toupper(group) == GROUP_A || toupper(group) == GROUP_B || toupper(group) == GROUP_C)){
     ;                                           // 组号合法
    }
    else {
        fprintf(stderr, "useradd: not a valid group in A, B, C\n");
        return;
    }

    // 不存在同名用户,且组号合法，进行添加
    strcpy(users[super_block.users_num].name, cur_cmd.argv[1]);
    strcpy(users[super_block.users_num].password, cur_cmd.argv[2]);
    users[super_block.users_num].group = cur_cmd.argv[3][0];

    super_block.users_num++;                         // 用户数加一

    fseek(file, USER_ADDRESS_START, SEEK_SET);
    fwrite(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);

    fprintf(stderr, "useradd: add user %s successfully.\n", cur_cmd.argv[1]);

}


/*
 * 删除用户，格式为
 * userdel user_name
 */
void userdel(){
    // root用户才可删除
    if(strcmp(cur_user.name, "root") != 0){
        fprintf(stderr, "userdel: only root can delete user.\n");
        return;
    }

    // 检查useradd格式
    if(cur_cmd.length != 2){
        fprintf(stderr, "userdel: userdel user_name.\n");
        return;
    }

    // 检查是否为不可删除用户
    if(strcmp(cur_cmd.argv[1], "root") == 0){
        fprintf(stderr, "userdel: can not delete user.\n");
        return;
    }

    fseek(file, USER_ADDRESS_START, SEEK_SET);
    struct User users[MAX_USER_NUM];
    fread(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);

    // 检查是否存在同名用户
    int i;
    for(i=0; i < super_block.users_num; i++){
        if(strcmp(users[i].name, cur_cmd.argv[1]) == 0){
            break;
        }
    }
    if(i >= super_block.users_num){
        fprintf(stderr, "userdel: No such user\n");
        return;
    }

    printf("point1\n");
    // 删除用户
    for(int j = i; j < super_block.users_num-1; j++){
        printf("point2\n");
        strcpy(users[j].name, users[j+1].name);
        strcpy(users[j].password, users[j+1].password);
        users[j].group = users[j+1].group;
    }
    super_block.users_num--;
    fseek(file, 0, SEEK_SET);
    fwrite(&super_block, sizeof(struct SuperBlock), 1, file);

    fseek(file, USER_ADDRESS_START, SEEK_SET);
    fwrite(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);

    fprintf(stderr, "userdel: delete %s successfully!\n", cur_cmd.argv[1]);

}

void touch(int parinoAddr,char name[],char buf[])	//touch命令创建文件，读入字符
{
//    printf("输入文件内容");
//    scanf("%s",buf);
    //	scanf("%s%s",name,buf);
    //先判断文件是否已存在。如果存在，打开这个文件并编辑
    if (strlen(name) >= MAX_NAME_SIZE) {
        printf("Exceeds the maximum filename length.\n");
        return;
    }
    //查找有无同名文件，有的话提示错误，退出程序。没有的话，创建一个空文件
    struct DirItem dirlist[16];    //临时目录清单
//    printf("111");
    //从这个地址取出inode
    struct Inode cur, fileInode;
    fseek(file, parinoAddr, SEEK_SET);
    fread(&cur, sizeof(struct Inode), 1, file);
    strcpy(cur.owner_name,cur_user.name);
    cur.mode=0b111111001;
//    printf("112");
    //判断文件模式。6为owner，3为group，0为other
    int filemode = 6;
//	if(strcmp(Cur_User_Name,cur.i_uname)==0 )
//		filemode = 6;
//	else if(strcmp(Cur_User_Name,cur.i_gname)==0)
//		filemode = 3;
//	else
//		filemode = 0;

    int i = 0, j;
    int dno;
    int fileInodeAddr = -1;    //文件的inode地址
    while (i < 128) {
        //128个目录项之内，可以直接在直接块里找
        dno = i / 16;    //在第几个直接块里

        if (cur.block_address[dno] == -1) {
            i += 16;
            continue;
        }
        fseek(file, cur.block_address[dno], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, file);
        fflush(file);
//        printf("113");
        //输出该磁盘块中的所有目录项
        for (j = 0; j < 16; j++) {
            if (strcmp(dirlist[j].item_name, name) == 0) {
                //重名，取出inode，判断是否是文件
                fseek(file, dirlist[j].inode_address, SEEK_SET);
                fread(&fileInode, sizeof(struct Inode), 1, file);
                if (fileInode.type == NORMAL) {    //是文件且重名，提示错误，退出程序
                    printf("file already exist.\n");
                    return;
                }
            }
            i++;
        }
    }
//    printf("114");
    //文件不存在，创建一个空文件
//	if( ((cur.i_mode>>filemode>>1)&1)==1){
    if(check_write_permission(cur)){
        //可写。可以创建文件
//        buf[0] = '\0';
        create(parinoAddr, name, buf);    //创建文件
    }
    else{
        printf("INSUFFICIENT PERMISSION: No write permission.\n");
        printf("warning\n");
        return ;
    }

}

bool create(int parinoAddr,char name[],char buf[])	//创建文件函数，在该目录下创建文件，文件内容存在buf
{
    if(strlen(name)>=MAX_NAME_SIZE){
        printf("Exceeds the maximum directory name length.\n");
        return false;
    }

    struct DirItem dirlist[16];	//临时目录清单

    //从这个地址取出inode
    struct Inode cur;
    fseek(file,parinoAddr,SEEK_SET);
    fread(&cur,sizeof(struct Inode),1,file);

    int i = 0;
    int posi = -1,posj = -1;	//找到的目录位置
    int dno;
    int cnt = cur.link_num+1;	//目录项数
    while(i<128){
        //128个目录项之内，可以直接在直接块里找
        dno = i/16;	//在第几个直接块里

        if(cur.block_address[dno]==-1){
            i+=16;
            continue;
        }
        fseek(file,cur.block_address[dno],SEEK_SET);
        fread(dirlist,sizeof(dirlist),1,file);
        fflush(file);

        //输出该磁盘块中的所有目录项
        int j;
        for(j=0;j<16;j++){

            if( posi==-1 && strcmp(dirlist[j].item_name,"")==0 ){
                //找到一个空闲记录，将新文件创建到这个位置
                posi = dno;
                posj = j;
            }
            else if(strcmp(dirlist[j].item_name,name)==0 ){
                //重名，取出inode，判断是否是文件
                struct Inode cur2;
                fseek(file,dirlist[j].inode_address,SEEK_SET);
                fread(&cur2,sizeof(struct Inode),1,file);
                if(cur2.type==NORMAL){	//是文件且重名，不能创建文件
                    printf("file already exist.\n");
                    buf[0] = '\0';
                    return false;
                }
            }
            i++;
        }

    }
    if(posi!=-1){	//之前找到一个目录项了
        //取出之前那个空闲目录项对应的磁盘块
        fseek(file,cur.block_address[posi],SEEK_SET);
        fread(dirlist,sizeof(dirlist),1,file);
        fflush(file);

        //创建这个目录项
        strcpy(dirlist[posj].item_name,name);	//文件名
        int chiinoAddr = ialloc();	//分配当前节点地址
        if(chiinoAddr==-1){
            printf("inode allocation failure\n");
            return false;
        }
        dirlist[posj].inode_address = chiinoAddr; //给这个新的目录分配的inode地址

        //设置新条目的inode
        struct Inode p;
//			p.i_ino = (chiinoAddr-INODE_ADDRESS_START)/INODE_SIZE;//caution
        p.update_time = time(NULL);

//			strcpy(p.i_uname,Cur_User_Name);
//			strcpy(p.i_gname,Cur_Group_Name);
        p.link_num = 1;	//只有一个文件指向
        p.group = cur_user.group;//集群名
        strcpy(p.owner_name,cur_user.name);//用户名
        //将buf内容存到磁盘块
        int k;
        int len = strlen(buf);	//文件长度，单位为字节
        p.size=len;
        for(k=0;k<len;k+=BLOCK_SIZE){	//最多8次，8个磁盘快，即最多5K
            //分配这个inode的磁盘块，从控制台读取内容
            int curblockAddr = balloc();
            if(curblockAddr==-1){
                printf("block allocation failure\n");
                return false;
            }
            p.block_address[k/BLOCK_SIZE] = curblockAddr;
            //写入到当前目录的磁盘块
            fseek(file,curblockAddr,SEEK_SET);
            fwrite(buf+k,BLOCK_SIZE,1,file);
        }


        for(k=len/BLOCK_SIZE+1;k<8;k++){
            p.block_address[k] = -1;
        }
        if(len==0){	//长度为0的话也分给它一个block
            int curblockAddr = balloc();
            if(curblockAddr==-1){
                printf("block allocation failure\n");
                return false;
            }
            p.block_address[k/BLOCK_SIZE] = curblockAddr;
            //写入到当前目录的磁盘块
            fseek(file,curblockAddr,SEEK_SET);
            fwrite(buf,BLOCK_SIZE,1,file);

        }
        p.size = len;


        p.type = NORMAL;
        p.mode = 0b111111001;

        //将inode写入到申请的inode地址
        fseek(file,chiinoAddr,SEEK_SET);
        fwrite(&p,sizeof(struct Inode),1,file);

        //将当前目录的磁盘块写回
        fseek(file,cur.block_address[posi],SEEK_SET);
        fwrite(dirlist,sizeof(dirlist),1,file);

        //写回inode
        cur.link_num++;
        fseek(file,parinoAddr,SEEK_SET);
        fwrite(&cur,sizeof(struct Inode),1,file);
        fflush(file);

        return true;
    }
    else
        return false;
}

bool del(int parinoAddr,char name[])		//删除文件函数。在当前目录下删除文件
{
    if(strlen(name)>=MAX_NAME_SIZE){
        printf("Exceeds the maximum directory name length.\n");
        return false;
    }

    //从这个地址取出inode
    struct Inode cur;
    fseek(file,parinoAddr,SEEK_SET);
    fread(&cur,sizeof(struct Inode),1,file);

    //取出目录项数
    int cnt = cur.link_num;
//    printf("%d\n",cur.link_num);
//    printf("%s\n",cur.owner_name);
//    printf("%d\n",cur.mode);
//    printf("%c\n",cur.group);
//    strcpy(cur.owner_name,"usera");
//    cur.mode=0b111111001;
    //判断文件模式。6为owner，3为group，0为other
//    int filemode=6;
//	if(strcmp(Cur_User_Name,cur.i_uname)==0 )
//		filemode = 6;
//	else if(strcmp(Cur_User_Name,cur.i_gname)==0)
//		filemode = 3;
//	else
//		filemode = 0;

    if(!check_write_permission(cur)){
        //没有写入权限
        printf("INSUFFICIENT PERMISSION: No write permission\n");
        return false;
    }

    //依次取出磁盘块
    int i = 0;
    while(i<128){	//小于128
        struct DirItem dirlist[16] = {0};

        if(cur.block_address[i/16]==-1){
            i+=16;
            continue;
        }
        //取出磁盘块
        int parblockAddr = cur.block_address[i/16];
        fseek(file,parblockAddr,SEEK_SET);
        fread(&dirlist,sizeof(dirlist),1,file);

        //找到要删除的目录
        int pos;
        for(pos=0;pos<16;pos++){
            struct Inode tmp;
            //取出该目录项的inode，判断该目录项是目录还是文件
            fseek(file,dirlist[pos].inode_address,SEEK_SET);
            fread(&tmp,sizeof(struct Inode),1,file);

            if( strcmp(dirlist[pos].item_name,name)==0){
                if( tmp.type==DIRECTORY ){	//找到目录
                    //是目录，不管
                }
                else{
                    //是文件

                    //释放block
                    int k;
                    for(k=0;k<8;k++)
                        if(tmp.block_address[k]!=-1)
                            bfree(tmp.block_address[k]);

                    //释放inode
                    ifree(dirlist[pos].inode_address);

                    //删除该目录条目，写回磁盘
                    strcpy(dirlist[pos].item_name,"");
                    dirlist[pos].inode_address = -1;
                    fseek(file,parblockAddr,SEEK_SET);
                    fwrite(&dirlist,sizeof(dirlist),1,file);
                    cur.link_num--;
                    fseek(file,parinoAddr,SEEK_SET);
                    fwrite(&cur,sizeof(struct Inode),1,file);

                    fflush(file);
                    return true;
                }
            }
            i++;
        }
    }

    printf("The file was not found!\n");
    return false;
}

void cls(){
    system("cls");
}

void wi(int parinoAddr,char name[],char buf[])
{
    struct Inode inode;
    fseek(file, parinoAddr, SEEK_SET);
    fread(&inode, sizeof(struct  Inode), 1, file);
    int dir_address = inode.block_address[0];
    struct DirItem dir_list[MAX_DIR_NUM];
    fseek(file, dir_address, SEEK_SET);
    fread(&dir_list, sizeof(struct DirItem)*MAX_DIR_NUM, 1, file);
    int dir_no;
    for(dir_no = 0; dir_no <MAX_DIR_NUM; dir_no++){
        if(strcmp(dir_list[dir_no].item_name, name)==0){
            break;
        }
    }
    if(dir_no < MAX_DIR_NUM){
        fseek(file, dir_list[dir_no].inode_address, SEEK_SET);
        fread(&inode, sizeof(struct Inode), 1, file);
        if(check_write_permission(inode) == false){
//            fprintf(stderr, "wi: permission denied.\n");
            return;
        }
    }

    if(strlen(name)>=MAX_NAME_SIZE){
        printf("exceed the max length.\n");
        return ;
    }

    //清空缓冲区
//    memset(buf,0,MAX_ARG_LENGTH);
    int maxlen = 0;	//到达过的最大长度

    //查找有无同名文件，有的话进入编辑模式，没有进入创建文件模式
    struct DirItem dirlist[16];	//临时目录清单

    //从这个地址取出inode
    struct Inode cur;
    struct Inode fileInode;
    fseek(file,parinoAddr,SEEK_SET);
    fread(&cur,sizeof(struct Inode),1,file);

    //判断文件模式。6为owner，3为group，0为other


    int i = 0,j;
    int dno;
    int fileInodeAddr = -1;	//文件的inode地址
    bool isExist = false;	//文件是否已存在
    while(i<128){
        //128个目录项之内，可以直接在直接块里找
        dno = i/16;	//在第几个直接块里

        if(cur.block_address[dno]==-1){
            i+=16;
            continue;
        }
        fseek(file,cur.block_address[dno],SEEK_SET);
        fread(dirlist,sizeof(dirlist),1,file);
        fflush(file);

        //输出该磁盘块中的所有目录项
        for(j=0;j<16;j++){
            if(strcmp(dirlist[j].item_name,name)==0 ){
                //重名，取出inode，判断是否是文件
                fseek(file,dirlist[j].inode_address,SEEK_SET);
                fread(&fileInode,sizeof(struct Inode),1,file);
                if( ((fileInode.mode>>9)&1)==0 ){	//是文件且重名，打开这个文件，并编辑
                    fileInodeAddr = dirlist[j].inode_address;
                    isExist = true;

                }
            }
            i++;
        }
    }
    //将buf内容写回磁盘块
    int k;
    int len = strlen(buf);	//文件长度，单位为字节
    for(k=0;k<len;k+=BLOCK_SIZE){	//最多10次，10个磁盘快，即最多5K
        //分配这个inode的磁盘块，从控制台读取内容
        int curblockAddr;
        if(fileInode.block_address[k/BLOCK_SIZE]==-1){
            //缺少磁盘块，申请一个
            curblockAddr = balloc();
            if(curblockAddr==-1){
                printf("block allocation failure.\n");
                return ;
            }
            fileInode.block_address[k/BLOCK_SIZE] = curblockAddr;
        }
        else{
            curblockAddr = fileInode.block_address[k/BLOCK_SIZE];
        }
        //写入到当前目录的磁盘块
        fseek(file,curblockAddr,SEEK_SET);
        fwrite(buf+k,BLOCK_SIZE,1,file);
        fflush(file);
    }
    //更新该文件大小
    fileInode.size = len;
    fileInode.update_time = time(NULL);
    fseek(file,fileInodeAddr,SEEK_SET);
    fwrite(&fileInode,sizeof(struct Inode),1,file);
    fflush(file);
}

// 若文件存在，返回i节点地址，否则返回-1
int Cd_judge_file(char skip_address[])    //目前是单级跳转,这是查询不是建立，不会对目录进行任何修改
{
    //由cur_dir查找当前的目录
    struct Inode temp;
    fseek(file,cur_dir.inode_address,SEEK_SET);
    fread(&temp,sizeof (struct Inode),1,file);
    fseek(file,temp.block_address[0],SEEK_SET);
    struct DirItem temp_dir[MAX_DIR_NUM];   //这里面装的是当前目前连接的最多16个文件或者目录,第一个是自己，第二个是父亲目录
    fread(&temp_dir,sizeof (temp_dir),1,file);   //连读16个struct DirItem
    int flag=0;
    for(int i=0;i<MAX_DIR_NUM;i++) {
        if (strcmp(temp_dir[i].item_name,skip_address)==0) {
            //cur_dir.inode_address = temp_dir[i].inode_address;
            flag = i;
            return temp_dir[i].inode_address;
            //printf("\ncurrent directory  i_node address:%d\n",cur_dir.inode_address);
        }
    }
    if(flag==0){
        printf("can not find this file!\n");
        return -1;
    }

}

void cat(char filename[])
{
    int sign=Cd_judge_file(filename);      //返回的是当前目录的物理块里修改文件的对应的i结点地址
//    memset(buffer,0,sizeof(buffer));
//    int maxlen=0;       //到达过的最大长度
    if(sign == -1)           //当前文件不存在
    {
        fprintf(stderr, "cat: file not exists.\n");
    }
    else{               //表明当前文件存在
        fseek(file,sign,SEEK_SET);
        struct Inode temp1;
        fread(&temp1,sizeof(temp1),1,file);
        if(temp1.type==NORMAL){
            fseek(file,temp1.block_address[0],SEEK_SET);
            char buffer[4096] = {'\0'};
            fread(&buffer,temp1.size,1,file);
            printf("file content:%s\n",buffer);
        }
        else{
            printf("this is a directory! can not open in a normal file way!\n");
        }
    }
}


/*
 * 输出超级块信息
 */
void super(){
    printf("SYSTEM:       Block size: %dB, Inode size: %dB\n", BLOCK_SIZE, sizeof(struct Inode));
    printf("              Number of blocks: %d\n", BLOCK_NUM);
    printf("DISK:         Super Block number: %d\n", 0);
    printf("              User Block number: %d\n", USER_ADDRESS_START/BLOCK_SIZE);
    printf("              Inode Blocks start number: %d\n", INODE_BLOCK_NO_START);
    printf("              Data Blocks start number: %d\n", INODE_BLOCK_NO_START+INODE_BLOCK_NUM);

    int empty_inode_num=0;
    // 统计空闲i节点的数量
    struct Inode inode;
    for(int i = INODE_ADDRESS_START; i < INODE_ADDRESS_END; i += INODE_SIZE){
        fseek(file, i, SEEK_SET);
        fread(&inode, sizeof(struct Inode), 1, file);
        if(inode.link_num == 0)
            empty_inode_num++;
    }
    printf("Empty Inodes: %d  /  All Inodes: %d\n", empty_inode_num, INODE_BLOCK_NUM*BLOCK_SIZE/sizeof(struct Inode));

    // 统计空闲块数量
    int empty_block_num = super_block.s_free[0];                              // 初始数量为空闲栈中空闲节点数量
    int start_grouper_addr = convert_block_num_to_address(super_block.s_free[1]);
    struct Grouper grouper;
    fseek(file, start_grouper_addr, SEEK_SET);
    fread(&grouper, sizeof(struct Grouper), 1, file);
    empty_block_num += grouper.s_free[0];
    while(grouper.s_free[1] != 0){
        fseek(file, convert_block_num_to_address(grouper.s_free[1]), SEEK_SET);
        fread(&grouper, sizeof(struct Grouper), 1, file);
        empty_block_num += grouper.s_free[0];
    }
    printf("Empty Data Blocks: %d  /  ALL Data Blocks: %d\n", empty_block_num, BLOCK_NUM-INODE_BLOCK_NUM-2);
}

/*
 * 帮助函数
 */
void help(){
    printf("ls                                      List directories and files.\n");
    printf("    -l                                  List with details.\n");
    printf("exit                                    Exit file system.\n");
    printf("cd dir_name                             Go to directory.\n");
    printf("mkdir dir_name                          Build a directory.\n");
    printf("rmdir dir_name                          Remove directory.\n");
    printf("useradd user_name password group        Create a user.\n");
    printf("userdel user_name                       Delete a user.\n");
    printf("touch file_name content                 Create file with content.\n");
    printf("rm file_name                            Delete file.\n");
    printf("cls                                     Clear screen.\n");
    printf("write file_name content                 Write file with content.\n");
    printf("cat file_name                           Read file content.\n");
    printf("logout                                  Logout account.\n");
    printf("super                                   Print system information\n");
    printf("listuser                                Print all users' information.\n");
    printf("chmod file_name permission              Change the file's permission.\n");
    printf("format                                  Format the file system.\n");
    printf("help                                    Command guide of system.\n");
}

/*
 * 输出用户信息
 */
void listuser(){
    fseek(file, USER_ADDRESS_START, SEEK_SET);
    struct User users[MAX_USER_NUM];
    fread(&users, sizeof(struct User)*MAX_USER_NUM, 1, file);
    printf("%20s %20s %20s\n", "User", "Password", "Group");
    for(int i=0; i< super_block.users_num; i++){
        if(strcmp(users[i].name, "") != 0){
            printf("%20s %20s %20c\n", users[i].name, users[i].password, users[i].group);
        }
    }
}


/*
 * 修改文件权限
 */
void change_permission(char filename[],char permission[]){      //传入的只能是3个字符
    int scar=1;
    int number=0;
    int power[3]={0};
    while(permission[number]!='\0'){
        if(permission[number]=='8'||permission[number]=='9'){scar=0;}
        number++;
    }
    if(number!=3)   {scar=0;}
    if(scar) {
        int sign = Cd_judge_file(filename);
        if (sign == -1) {
            printf("can not find this file!\n");
        } else {
            for (int i = 0; i <= 2; i++) {
                switch (permission[i]) {
                    case '0':
                        power[2 - i] = 0b000;
                        break;
                    case '1':
                        power[2 - i] = 0b001;
                        break;
                    case '2':
                        power[2 - i] = 0b010;
                        break;
                    case '3':
                        power[2 - i] = 0b011;
                        break;
                    case '4':
                        power[2 - i] = 0b100;
                        break;
                    case '5':
                        power[2 - i] = 0b101;
                        break;
                    case '6':
                        power[2 - i] = 0b110;
                        break;
                    case '7':
                        power[2 - i] = 0b111;
                        break;
                    default:
                        printf("illegal input permission number!\n");
                }
            }
        }
        fseek(file, sign, SEEK_SET);
        struct Inode temp1;
        fread(&temp1, sizeof(temp1), 1, file);
        if (strcmp(cur_user.name, "root") == 0) {
            temp1.mode = (power[2] << 6) | (power[1] << 3) | (power[0]);
            fseek(file, sign, SEEK_SET);
            fwrite(&temp1, sizeof(temp1), 1, file);
        } else {
            printf("you are not root user,can't change the permission!\n");
        }
    }
    else{
        printf("input is wrong!\n");
    }

}

/*
 * 格式化磁盘
 */
bool formattor(){
    if(strcmp(cur_user.name, "root")!=0){
        fprintf(stderr, "format: only root user can format.\n");
        return false;
    }
    format();
    fprintf(stderr, "format: format successfully.\n");
    return true;
}