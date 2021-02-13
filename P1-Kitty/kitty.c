#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 4096

int main(int argc, char *argv[]){

	int fin;
	int fout;
	char* inName;
	char* outName;
	int readTime;
	int writeTime;
	int byteTransfered;
	int byteRead;
	int byteWrite;
	char buf[BUFSIZE];
	int mode;
	
	char c;
	int i; // use to track which argument is handling
	int isBinary;
	
	isBinary = 0;
	mode = 1;

	if(argc >= 2){

		if(strcmp(argv[1],"-o") == 0){
			//check if output file is specified
			outName = argv[2];
			if(argc != 3){
				i = 3;
			}else{
				i = 2;// to ensure enter the loop
				mode = 2;
			}
			if((fout = open(outName,O_WRONLY|O_CREAT|O_TRUNC, 0666)) <= 0){
				fprintf(stderr,"ERROR: Fail to open/create %s for writing with error %s\n", outName, strerror(errno));
				return -1;	
			}

		}else{
			outName = "<standard output>";
			fout = 1;
			i = 1;
		}
	}else{
		i = 0;
		fout = 1;
		outName = "<standard output>";
	}
		
	for(i ; i < argc; i++){
		// read input filenames	
		inName = argv[i];
		
		byteTransfered = 0;
		readTime = 0;
		writeTime = 0;
		isBinary = 0;

		if(strcmp(inName, "-") == 0 || i == 0 || mode == 2){
			inName = "<standard input>";
			fin = 0;
		}else{
			if((fin = open(inName, O_RDONLY)) < 0  ){
				if(errno == ENOENT){
					// a common error can happen
					fprintf(stderr,"ERROR: %s is not a name of a valid file with error %s\n", inName,strerror(errno));
					return -1;
				}else{
					fprintf(stderr, "ERROR: an error occurred when opening %s with error %s\n", inName, strerror(errno));
					return -1;
				}	
			}
		}
		
		while((byteRead = read(fin, buf, BUFSIZE)) > 0){
			byteTransfered += byteRead;
			if(checkBinary(buf, byteRead) == 1){
				isBinary = 1;
			}
			if((byteWrite = write(fout, buf, byteRead)) < byteRead){
				int s = 0;
				int j;
				for(j = byteWrite; j < byteRead; j++){
					buf[s] = buf[j];
					s++;
				}
				if(write(fout, buf, (byteRead - byteWrite)) < (byteRead - byteWrite)){
					//Partial write again
					fprintf(stderr, "ERROR: Only a part of data is wrote to %s from %s", inName, outName);
					return -1;
				}
				writeTime++;
			}else if(byteWrite < 0){
				fprintf(stderr, "ERROR: An error occurred when writing to %s with the error %s\n", outName, strerror(errno));
				return -1;
			}
			readTime++;
			writeTime++;
		}	
		readTime++;	

		if(byteRead < 0){
			printf("%d", fin);
			fprintf(stderr, "ERROR: An error occurred when reading from %s with the error %s\n", inName, strerror(errno));
			return -1;
		}			
		fprintf(stderr, "%d bytes is transfered; %d times of read from %s; %d times of write to %s\n ", byteTransfered, readTime, inName, writeTime,outName);
		
		if(fin != 0){
			if(close(fin) < 0){
				fprintf(stderr, "ERROR: Fail to close %s with error %s\n", inName, strerror(errno));
				return -1;
			}
		}

		if(isBinary == 1){
			fprintf(stderr, "WARN: %s is binary file\n", inName);
		}
	}

	if(fout != 1){
		if(close(fout) < 0){
			fprintf(stderr, "ERROR: Fail to close %s with error %s\n", outName, strerror(errno));
			return -1;
		}
	}
	return 0;
}

int checkBinary(char buf[], int size){
	int ii;
	for( ii = 0; ii < size; ii++){
		if((buf[ii] < 32) ||(buf[ii] >= 127) ){
			return 1;
		}
	}
	return 0;
}
