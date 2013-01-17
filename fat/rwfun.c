/*
*	This file has read & write functions
*	These functions may use by other C file
*	in different use,change some read & write functions
*	By Ming		2009-1
*/
#include "rwfun.h"

int fd;//fd for read and write
WORD SectorSize;

int initRW(const char *pathname){
	//return -1 for failed,else return fd
	return (fd=open(pathname,O_RDONLY));//init device for read write
}

int readRW(void *buf,int count){
	//return -1 for failed,else return num of read bytes
	return read(fd,buf,count);//read data
}

int writeRW(const void *buf,int count){
	//return -1 for failed,else return num of write bytes
	return write(fd,buf,count);//write data
}

int seekRW(OFF_T sector,OFF_T offset){//real offset is : sector * SectorSize + offset
	//return -1 for failed
	WORD tmp=SectorSize;
	if(lseek64(fd,0,SEEK_SET)==-1){//back to start of file
		return -1;
	}
	while((tmp--)>0){
		if(lseek64(fd,sector,SEEK_CUR)==-1)
			return -1;
			
	}
	return lseek64(fd,offset,SEEK_CUR);//seek data
}

int seekreadRW(OFF_T sector,OFF_T offset,void *buf,int count){//read with seek
	if(seekRW(sector,offset)==-1){//frist seek
		return -1;
	}
	return readRW(buf,count);//second read
}

int seekwriteRW(OFF_T sector,OFF_T offset,void *buf,int count){//write with seek
	if(seekRW(sector,offset)==-1)//frist seek
		return -1;
	return writeRW(buf,count);//second write
}

void setSectorSize(WORD Size){//set the sector size
	SectorSize=Size;
}

