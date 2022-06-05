//#include <iostream>
#include "file.h"

int main() {
//    printf("size of superblock: %d, inode: %d, user: %d\n", sizeof(struct SuperBlock), sizeof(struct Inode), sizeof(struct User));

    init();
    while(true) {
        login();
        sh();
    }
}