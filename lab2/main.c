#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include "similarity.h"

#define SECTOR_SIZE 512             //扇区大小（字节）
#define ENTRY_SIZE 32               //根目录条目大小（字节）
#define NAME_SIZE 13                //文件夹和文件的名称大小（字节）
#define ROOT_ENTRY_NUM 14           //根目录最大文件数

struct File { //文件
    //名称
    char name[NAME_SIZE];
    //条目内容
    char entry[ENTRY_SIZE];
    //数据
    char data[SECTOR_SIZE];
    struct Directory *parent;//父目录
};

struct Directory { //文件夹
    //名称，条目内容，数据
    char name[NAME_SIZE], entry[ENTRY_SIZE], data[SECTOR_SIZE];
    //父目录
    struct Directory *parent;
    //所有文件
    struct File *files[100];
    //所有子文件夹
    struct Directory *dirs[100];
    int fileNum, dirNum;//文件数、文件夹数
};

/*读取数据，对FAT、ROOT、data进行赋值*/
void readData();

/*获得指定FAT项的值，参数为一个簇号*/
int getFAT(int);

/*获得数据，第一个参数为数据，第二个参数为条目*/
void getData(char *, char *);

/*拷贝字符串*/
void copy(char *destination, const char *src, int length);

/*列出根目录下的所有文件（夹）*/
void listRoot();

/*列出指定目录下的所有文件（夹）*/
void list(struct Directory *);

/*判断是否是文件*/
bool isFile(const char *);

/*判断是否是文件夹*/
bool isDirectory(const char *);

/*判断输入文件夹路径是否合法,返回文件夹在Dirs的位置，不存在返回-1*/
int isLegalDir(char *);

/*判断指定目录下是否存在文件，第一个参数为目录，第二个参数为待判断的文件，返回文件所在位置，不存在返回-1*/
int containsFile(struct Directory *, char *);

/*判断指定目录下是否存在文件夹，返回文件夹在Dirs的位置，不存在返回-1*/
int containsDir(const char *name);

/*显示文件内容*/
void showFile(char *);

/*显示文件夹内所有子文件夹及文件*/
void showDir(char *);

/*统计*/
void count(char *);

/*统计文件夹内子文件夹及文件数目，第二个参数为需要输出的tab数量*/
void countDir(struct Directory *, int);

/*获取文件（文件夹）名称, 返回名称长度;第一个参数为名称，第二个参数为条目内容*/
int getName(char *, const char *);

/*打印文件内容*/
void printFileContent(const struct File *);

/*打印指定簇中的内容，参数为簇号*/
void printData(int);

/*打印文件名*/
void printFileName(const struct File *);

/*打印文件夹名*/
void printDirectoryName(const struct Directory *);

/*打印空文件夹名*/
void printEmptyDirName(const struct Directory *);

/*打印文件名*/
extern int my_printFile(char *, int);

/*打印文件夹名*/
extern int my_printDirectory(char *, int);

unsigned char FAT[9 * SECTOR_SIZE];         //FAT1内容,不涉及Ascii，使用无符号char
char ROOT[ROOT_ENTRY_NUM * SECTOR_SIZE];    //根目录内容
char DATA[2846 * SECTOR_SIZE];              //data区内容
struct Directory root;                      //根目录
struct Directory *Dirs[100];                 //所有文件夹
int dirNum = 0;                             //文件夹数量

int main(void) {
    readData();
    listRoot();
    printf("%s\n", "Input any command to continue or 0 to quit:");

    char input[100];
    int length = 0;
    while (true) {
        fgets(input, 100, stdin);
        if (input[0] == '\n') {
            continue;
        }
        input[strlen(input) - 1] = '\0';//去掉'\n'
        length = strlen(input);

        if (input[0] == '0') {
            return 0;
        }

        char *command;
        command = strtok(input, " ");
        strcat(command, "\0");
        int i = getSimilarity("count", command);
        if (strcmp("count", command) == 0) {
            count(strtok(NULL, " "));
        } else if (i == 1) {
            printf("Didn't find the command '%s', Do you mean 'count'?\n", command);
        } else if (input[length - 1] == '/') {//行尾包含'/'和'\n'
            showDir(input);
        } else {
            showFile(input);
        }
    }
}

void readData() {
    FILE *file = fopen("a.img", "r");
    char sector[SECTOR_SIZE];                //存储一个扇区

    memset(FAT, '\0', sizeof(FAT));
    memset(ROOT, '\0', sizeof(ROOT));
    memset(DATA, '\0', sizeof(DATA));

    fread(sector, SECTOR_SIZE, 1, file);    //引导扇区
    fread(FAT, sizeof(FAT), 1, file);       //FAT1
    for (int i = 0; i < 9; ++i) {//FAT2,丢弃
        fread(sector, SECTOR_SIZE, 1, file);
    }
    fread(ROOT, sizeof(ROOT), 1, file);     //根目录
    fread(DATA, sizeof(DATA), 1, file);     //数据

    fclose(file);
}

void listRoot() {
    char entry[ENTRY_SIZE];         //一个条目
    //1.读取条目
    //2.文件--添加文件
    //3.文件夹--添加到Dirs及根目录子文件夹
    for (int i = 0; i < sizeof(ROOT);) {
        for (int j = 0; j < ENTRY_SIZE; ++j, ++i) {
            entry[j] = ROOT[i];
        }
        char name[NAME_SIZE];
        getName(name, entry);
        if (isFile(entry)) {//文件
            struct File *f;
            f = (struct File *) malloc(sizeof(struct File));
            strcpy(f->name, name);
            copy(f->entry, entry, ENTRY_SIZE);
            root.files[root.fileNum++] = f;
            printFileName(f);
        } else if (isDirectory(entry)) {//目录
            struct Directory *temp;
            temp = (struct Directory *) malloc(sizeof(struct Directory));
            strcpy(temp->name, name);
            copy(temp->entry, entry, ENTRY_SIZE);
            Dirs[dirNum++] = temp;
            root.dirs[root.dirNum++] = temp;
            list(temp);
            if (temp->dirNum == 0 && temp->fileNum == 0) {//空文件夹
                printEmptyDirName(temp);
            }
        }
    }
}

void list(struct Directory *dir) {
    char entry[ENTRY_SIZE];         //一个条目
    //1.读取条目
    //2.文件--打印当前文件夹名
    //3.文件夹--
    //  3.1读取数据，递归
    //  3.2若目录为空，打印空文件夹名
    getData(dir->data, dir->entry);
    for (int i = 0; i < SECTOR_SIZE;) {
        for (int j = 0; j < ENTRY_SIZE; ++j, ++i) {
            entry[j] = dir->data[i];
        }
        char name[NAME_SIZE];
        getName(name, entry);
        if (isFile(entry)) {//文件
            struct File *f;
            f = (struct File *) malloc(sizeof(struct File));
            strcpy(f->name, name);
            copy(f->entry, entry, ENTRY_SIZE);
            f->parent = dir;
            dir->files[dir->fileNum++] = f;
            printFileName(f);
        } else if (isDirectory(entry)) {//目录
            struct Directory *temp;
            temp = (struct Directory *) malloc(sizeof(struct Directory));
            strcpy(temp->name, name);
            copy(temp->entry, entry, ENTRY_SIZE);
            temp->parent = dir;
            Dirs[dirNum++] = temp;
            dir->dirs[dir->dirNum++] = temp;
            list(temp);
            if (temp->dirNum == 0 && temp->fileNum == 0) {
                printEmptyDirName(temp);
            }
        }
    }
}

bool isFile(const char *entry) {
    if (entry[11] != 0x20) {//若不是文件，则返回false
        return false;
    }
    if (!isalpha(entry[0]) && entry[0] != '.') {//若首字母非法，则返回false
        return false;
    }

    return true;
}

bool isDirectory(const char *entry) {//若不是文件夹，则返回false
    if (entry[11] != 0x10) {
        return false;
    }
    if (!isalpha(entry[0])) {//若首字母非法，则返回false
        return false;
    }

    return true;
}

int containsFile(struct Directory *dir, char *string) {
    for (int i = 0; i < dir->fileNum; ++i) {
        if (strcasecmp(dir->files[i]->name, string) == 0) {
            return i;
        }
    }

    return -1;
}

int containsDir(const char *name) {
    for (int i = 0; i < dirNum; i++) {
        if (strcasecmp(Dirs[i]->name, name) == 0) {
            return i;
        }
    }

    return -1;
}

int getName(char *name, const char *entry) {
    if (!isFile(entry) && !isDirectory(entry)) {
        return 0;
    }

    int length = 0;
    memset(name, '\0', NAME_SIZE);
    for (int i = 0; i < 8; ++i) {
        if (entry[i] != ' ') {
            name[length++] = entry[i];
        }
    }
    if (isDirectory(entry)) {
        return length;
    }

    name[length++] = '.';
    for (int i = 8; i < 11; ++i) {
        name[length++] = entry[i];
    }

    return length;
}

int getFAT(int FATNum) {
    //簇号-偏移对应关系：2-3,3-3,4-6,5-6,6-9,7-9,8-12
    int index = (FATNum / 2) * 3;                       //偏移量
    int result = 0;                                   //FAT项

    if (FATNum % 2 == 1) {//奇数，高12位
        result = FAT[index + 2] * 16 + (FAT[index + 1] >> 4);
    } else {//偶数，低12位
        result = FAT[index] + (char) (FAT[index + 1] << 4) * 16;
    }

    return result;
}

void getData(char *entryData, char *entry) {
    int index = entry[27] * 256 + entry[26];
    index = (index - 2) * SECTOR_SIZE;

    for (int i = 0; i < SECTOR_SIZE; ++i) {
        entryData[i] = DATA[i + index];
    }
}

void printFileName(const struct File *file) {
    char name[NAME_SIZE];
    int length = getName(name, file->entry);
    name[length++] = '\n';

    if (file->parent != NULL) {
        printDirectoryName(file->parent);
    }

    my_printFile(name, length);
}

void printDirectoryName(const struct Directory *dir) {
    char name[NAME_SIZE];
    int length = getName(name, dir->entry);
    name[length++] = '/';

    if (dir->parent != NULL) {
        printDirectoryName(dir->parent);
    }

    my_printDirectory(name, length);
}

void printEmptyDirName(const struct Directory *dir) {
    char name[NAME_SIZE];
    int length = getName(name, dir->entry);
    name[length++] = '\n';//空文件夹最后无需‘/’，输出换行符

    if (dir->parent != NULL) {
        printDirectoryName(dir->parent);
    }

    my_printDirectory(name, length);
}

int isLegalDir(char *path) {
    char separator[] = "/";
    char *result = NULL;
    result = strtok(path, separator);

    int index = 0;
    while (result != NULL) {
        index = containsDir(result);
        if (index == -1) {
            return index;
        }
        result = strtok(NULL, separator);
    }

    return index;
}

void showFile(char *path) {
    char *name = strrchr(path, '/');

    if (name == NULL) {//根目录下
        int index = containsFile(&root, path);
        if (index != -1) {
            printFileContent(root.files[index]);
        } else {
            printf("%s\n", "Unknown file!");
        }
    } else {
        char temp[strlen(name)];
        for (int i = 0; i < strlen(name); ++i) {//删除'/'
            temp[i] = name[i + 1];
        }
        name[0] = '\0';//截断path，取出文件路径

        int i = isLegalDir(path);
        if (i == -1) {
            printf("%s\n", "Unknown file!");
        } else {
            int j = containsFile(Dirs[i], temp);
            if (j == -1) {
                printf("%s\n", "Unknown file!");
            } else {
                printFileContent(Dirs[i]->files[j]);
            }
        }
    }
}

void showDir(char *input) {
    if (strcmp(input, "/") == 0) {//根目录
        listRoot();
        return;
    }
    int index = isLegalDir(input);
    if (index != -1) {
        list(Dirs[index]);
    } else {
        printf("Unknown directory!\n");
    }
}

void count(char *path) {
    if (path == NULL) {
        printf("No input directory!\n");
        return;
    }
    if (strcmp("/", path) == 0) {
        countDir(&root, 1);
        return;
    }
    int index = isLegalDir(path);
    if (index != -1) {//存在文件夹
        countDir(Dirs[index], 1);
    } else {//不存在
        printf("%s is not a directory!\n", path);
    }
}

void countDir(struct Directory *dir, int num) {
    if (dir == &root) {
        printf("/ : ");
    } else {
        printf("%s : ", dir->name);
    }
    if (dir->fileNum == 0 || dir->fileNum == 1) {
        printf("%d file, ", dir->fileNum);
    } else {
        printf("%d files, ", dir->fileNum);
    }
    if (dir->dirNum == 0 || dir->dirNum == 1) {
        printf("%d directory\n", dir->dirNum);
    } else {
        printf("%d directories\n", dir->dirNum);
    }

    for (int i = 0; i < dir->dirNum; ++i) {
        for (int j = 0; j < num; ++j) {
            printf("%c", '\t');
        }
        countDir(dir->dirs[i], num + 1);
    }
}

void printFileContent(const struct File *file) {
    int index = file->entry[27] * 256 + file->entry[26];//簇号
    printData(index);
}

void printData(int index) {
    for (int i = (index - 2) * SECTOR_SIZE; i < (index - 1) * SECTOR_SIZE; ++i) {
        printf("%c", DATA[i]);
    }
    if (getFAT(index) < 0xff7) {
        printData(getFAT(index));
    }
}

void copy(char *destination, const char *src, int length) {
    for (int i = 0; i < length; ++i) {
        destination[i] = src[i];
    }
}
