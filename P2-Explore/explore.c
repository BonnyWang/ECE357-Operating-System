#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define NFREE 0
#define NFIFO 1
#define NCHR 2
#define NDIR 3
#define NBLK 4
#define NFIL 5
#define NSYM 6
#define NSOCK 7

#define SSIZE 0
#define SDISK 1
#define SLINK 2
#define PDIR 3
#define ILINK 4

#define MAX 4096

int checkType(struct stat st, char* pathName, int nTypes[], int statFile[]);
int exploreDir(char *dirName,struct stat st, int nTypes[], int statFile[]);
int check_DirName(char* Name);
int checkLink(int links,int statFile[]);
int checkSymlink(char *path, int statFile[]);

int main(int argc, char *argv[]){
    
    int nTypes[8];
    int statFile[2];

    int inLink1more;
    int invalid_Symlink;
    int problem_Dir;

    struct stat st;
    char *pathName;

    int exit = 1;
    int i;

    for(i = 0; i < 8; i++){
        nTypes[i] = 0;
    }
    for(i = 0; i < 5; i++){
        statFile[i] = 0;
    }


    pathName = "";
    
    if(argc < 2){
        //No specified argument, path is equal to current
        pathName = "./";
    }else if (argc == 2){
        if(argv[1][0] == '/'){
            fprintf(stderr,"ERROR: please run this program at the same mounted volume.\n");
            return -1;
        }else{
            pathName = argv[1];
        }
    }else{
        fprintf(stderr,"ERROR: Too many argument! Please only enter one path to explore.\n");
        return -1;
    }
        
    if(lstat(pathName, &st) < 0){
        fprintf(stderr,"ERROR: Unable to obtain the stat for %s because %s\n", pathName, strerror(errno));
    }

    checkType(st, pathName,nTypes,statFile);

    printf("--------------------------------------------------\n");
    printf("| Directory         |    %20d    |\n",nTypes[NDIR]);
    printf("| File              |    %20d    |\n",nTypes[NFIL]);
    printf("| FIFO              |    %20d    |\n",nTypes[NFIFO]);
    printf("| Socket            |    %20d    |\n",nTypes[NSOCK]);
    printf("| Symlink           |    %20d    |\n",nTypes[NSYM]);
    printf("| CharDevice        |    %20d    |\n",nTypes[NCHR]);
    printf("| BlkDevice         |    %20d    |\n",nTypes[NBLK]);
    printf("| File Size         |    %20d    |\n",statFile[SSIZE]);
    printf("| Disk Allocated    |    %20d    |\n",statFile[SDISK]);
    printf("| More then 1 link  |    %20d    |\n",statFile[SLINK]);
    printf("| Problematic Dir   |    %20d    |\n",statFile[PDIR]);
    printf("| Invalid Symlink   |    %20d    |\n",statFile[ILINK]);
    printf("--------------------------------------------------\n");

    return 0;
}

int exploreDir(char *dirName,struct stat st, int nTypes[], int statFile[]){

    DIR *dirp;
    struct dirent *de;

    char subPathName[MAX];

    if(strlen(dirName) > MAX){
        fprintf(stderr,"Error: the path name is too long!");
    }else{
        strcpy(subPathName,dirName);
    }
 
    if (!(check_DirName(dirName) < 0) ){

        if (!(dirp=opendir(dirName))){
            fprintf(stderr,"ERROR:Can not open directory %s : %s\n",dirName,strerror(errno));
            return -1;
        }

        while(de = readdir(dirp)){
            
            if(!(check_DirName(de->d_name) < 0)){

                if((strlen(subPathName)+strlen(de->d_name)) > MAX){
                    fprintf(stderr,"Error: the path name is too long! %s/%s", subPathName, de->d_name);
                }else{
                    if(subPathName[strlen(subPathName)-1] != '/') {
                        strcat(subPathName,"/");
                    }
                    if(check_DirName(de->d_name) == 0){
                        statFile[PDIR]++;
                    }
                    strcat(subPathName,de->d_name);
                    // printf("Subpath : %s\n", subPathName);
                    if(lstat(subPathName, &st) < 0){
                        fprintf(stderr,"ERROR: Unable to obtain the stat for %s\n because %s\n", subPathName, strerror(errno));
                    }else{
                        checkType(st, subPathName, nTypes, statFile);
                    }

                    //Return to the HomeDir to check next element
                    strcpy(subPathName, dirName);
                }

            }
       
        }
        if ((de == NULL) & errno){
            fprintf(stderr,"Error reading directory %s:%s\n",dirName,strerror(errno));
        }
        if(closedir(dirp) < 0){
            fprintf(stderr,"Error closing directory %s:%s\n",dirName,strerror(errno));
        }
    }
}

int checkType(struct stat st, char* pathName, int nTypes[], int statFile[]){
    if ((strcmp(pathName, "..") != 0) && (strcmp(pathName, ".") != 0)) {
        if((st.st_mode & S_IFMT) == S_IFDIR){
                nTypes[NDIR]++;
                exploreDir(pathName,st,nTypes,statFile);
        }else{
            checkLink(st.st_nlink, statFile);
            if((st.st_mode & S_IFMT) == S_IFCHR){
                nTypes[NCHR]++;
            }else if((st.st_mode & S_IFMT) == S_IFBLK){
                nTypes[NBLK]++;
            }else if((st.st_mode & S_IFMT) == S_IFSOCK){
                nTypes[NSOCK]++;
            }else if((st.st_mode & S_IFMT) == S_IFIFO){
                nTypes[NFIFO]++;
            }else if((st.st_mode & S_IFMT) == S_IFREG){
                nTypes[NFIL]++;
                statFile[SSIZE] += st.st_size;
                statFile[SDISK] += st.st_blocks;
            }else if((st.st_mode & S_IFMT) == S_IFLNK){
                nTypes[NSYM]++;
                checkSymlink(pathName, statFile);
            }
        } 
    }
    return 0;
}

int check_DirName(char* Name){
    char c;
    int i;
 
    if((strcmp(Name, ".") == 0) || (strcmp(Name, "..") == 0)){
        return -1;
    }

    for(i = 0; i < strlen(Name); i++){
        c = Name[i];
        if(!(isprint(c)||isspace(c))){
            return 0;
        }
    }

    return 1;
}

int checkLink(int links,int statFile[]){
    if(links > 1){
        statFile[SLINK]++;
        return 1;
    }
    return -1;
}

int checkSymlink(char *path, int statFile[]){
    struct stat st;
    if(stat(path, &st) < 0){
        statFile[ILINK]++;
        return 0;
    }
    return 1;
}