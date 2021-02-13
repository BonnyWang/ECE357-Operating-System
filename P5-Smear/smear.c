#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char* argv[]){

    int i = 0;
    int fd;
    struct stat mstat;
    char *aptr;
    char *itr;

    if(argc < 4){
        fprintf(stderr,"ERROR: Please use the program with correct format!\n");
        return -1;
    }else if(strlen(argv[1]) != strlen(argv[2])){
        fprintf(stderr,"ERROR: Target string must be the same length as the replacement string!\n");
        return -1;
    }

    for(i = 3; i < argc; i++){
        if((fd = open(argv[i], O_RDWR)) < 0){
            fprintf(stderr, "ERROR: Fail to open %s because %s", argv[i], strerror(errno));
            continue;
        }

        if(fstat(fd, &mstat)<0 ){
			fprintf(stderr,"ERROR:Fail to obtain the stat information of %s because %s\n", argv[i], strerror(errno));
			continue;
		}

        if((aptr = mmap(NULL,mstat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED){
			fprintf(stderr,"ERROR: Fail to mmap for %s becuase %s\n", argv[i], strerror(errno));
			continue;
		}
        
        itr = aptr;
		while(itr < (aptr+mstat.st_size-strlen(argv[1]) + 1)){
			if(strncmp(itr,argv[1],strlen(argv[1]))==0){
                memcpy(itr,argv[2],strlen(argv[1]));
            }
			itr++;
		}

        //synchronize a file with a memory map
        if(msync(aptr,mstat.st_size,MS_SYNC) < 0 ){
			fprintf(stderr,"ERROR: Fail to sync for %s because %s\n", argv[i], strerror(errno));
            continue;
		}

        if(munmap(aptr,mstat.st_size) < 0 ){
			fprintf(stderr,"ERROR: Fail to unmap %s because %s\n", argv[i],strerror(errno));
			continue;
		}

        if( close(fd)<0 ){
			fprintf(stderr,"ERROR: Fail to close %s because %s\n", argv[i],strerror(errno));
			continue;
		}
    }

    return 0;
}