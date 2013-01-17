/*
*	This file has FAT functions
*	These functions may use by other C file
*	By Ming		2009-1
*/
#include "fat.h"

BYTE FATType;//flag of file system type,0--FAT32,1--FAT16,2--FAT12
DWORD FATSz;//the FAT size
DWORD FirstDataSector;//first data sector,it is the start sector of data
WORD RsvdSecCnt;//Number of reserved sectors
WORD BytsPerSec;//Count of bytes per sector
BYTE SecPerClus;//Number of sectors per allocation unit
DWORD RootClus;//Cluster number of root dir
WORD RootDirSectors;//count of root directory sectors



int GetBPB(struct BPB * BPBadd){
	if(seekreadRW(0,0,(void*)(BPBadd),36)==-1){//Get frist 36 bytes BPB information
		return -1;
	}
	if(*((WORD*)(*BPBadd).BPB_FATSz16)!=0){//BPB_FATSz16=0 means file system is FAT32,BPB_FATSz16!=1 means FAT16 or FAT12
		if(seekreadRW(0,36,(void*)(&((*BPBadd).BS_DrvNum)),26)==-1)//Get left 26 bytes BPB information(FAT16/12)
			return -1;
	}
	else{
		if(seekreadRW(0,36,(void*)(&((*BPBadd).BPB_FATSz32)),54)==-1)//Get left 54 bytes BPB information(FAT32)
			return -1;
	}
	setSectorSize(*((WORD*)(*BPBadd).BPB_BytsPerSec));//set sector size(for seek function)
}

BYTE GetFATType(struct BPB * BPBadd){//Get FAT type and init some information (this function may be call!!!)
	DWORD TotSec;//the total sector num
	DWORD DataSec;//data sector
	DWORD CountofClusters;//the count of data clusters
	//*******************save some useful information*******************//
	RsvdSecCnt=*((WORD*)(*BPBadd).BPB_RsvdSecCnt);
	BytsPerSec=*((WORD*)(*BPBadd).BPB_BytsPerSec);
	SecPerClus=(*BPBadd).BPB_SecPerClus;
	//*******************get FAT type*******************//
	if(*((WORD*)(*BPBadd).BPB_FATSz16)!=0){
		FATSz=*((WORD*)(*BPBadd).BPB_FATSz16);//Get FAT Size
		if((TotSec=*((WORD*)(*BPBadd).BPB_TotSec16))==0){//Get Total Sectors
			TotSec=*((DWORD*)(*BPBadd).BPB_TotSec32);
		}
	}
	else{
		FATSz=*((DWORD*)(*BPBadd).BPB_FATSz32);//Get FAT Size
		TotSec=*((DWORD*)(*BPBadd).BPB_TotSec32);//Get Total Sectors
	}
	//RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec
	RootDirSectors=( *((WORD*)(*BPBadd).BPB_RootEntCnt) * 32 + (BytsPerSec-1) ) / BytsPerSec;
	//FirstDataSector = BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors
	FirstDataSector=*((WORD*)(*BPBadd).BPB_RsvdSecCnt) + ((*BPBadd).BPB_NumFATs * FATSz) + RootDirSectors;
	//DataSec = TotSec - (BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors)
	DataSec=TotSec-FirstDataSector;
	//CountofClusters = DataSec / BPB_SecPerClus;
	CountofClusters=DataSec / SecPerClus;

	if(CountofClusters<4085){
		FATType=FAT12;//Volume is FAT12
	}
	else if(CountofClusters<65525){
		FATType=FAT16;//Volume is FAT16
		RootClus=0;
	}
	else{
		FATType=FAT32;//Volume is FAT32
		RootClus=*((WORD*)(*BPBadd).BPB_RootClus);
	}

	return FATType;
}

int GetFAT32FSI(WORD FSInfo,struct FSI * FSIadd){//Get FAT32 FSInfo,FSInfo may get from BPB(FAT32 only)
	if(FATType==FAT32){
		if(seekreadRW(FSInfo,0,(void*)(&((*FSIadd).FSI_LeadSig)),4)==-1){//Get FSI_LeadSig
			return -1;
		}
		if(seekreadRW(FSInfo,484,(void*)(&((*FSIadd).FSI_StrucSig)),4)==-1){//Get FSI_LeadSig
			return -1;
		}
		if(seekreadRW(FSInfo,488,(void*)(&((*FSIadd).FSI_Free_Count)),4)==-1){//Get FSI_LeadSig
			return -1;
		}
		if(seekreadRW(FSInfo,492,(void*)(&((*FSIadd).FSI_Nxt_Free)),4)==-1){//Get FSI_LeadSig
			return -1;
		}
		if(seekreadRW(FSInfo,508,(void*)(&((*FSIadd).FSI_TrailSig)),4)==-1){//Get FSI_LeadSig
			return -1;
		}
		return 0;
	}
	else{
		return -2;//File system is not FAT32
	}
}

DWORD GetNextClus(DWORD N){
	DWORD FATOffset;
	DWORD ThisFATSecNum;
	DWORD ThisFATEntOffset;
	BYTE readbuf[4];
	if(FATType==FAT32){
		FATOffset=N*4;
	}
	else if(FATType==FAT16){
		FATOffset=N*2;
	}
	ThisFATSecNum=RsvdSecCnt + ( FATOffset / BytsPerSec );
	ThisFATEntOffset=FATOffset % BytsPerSec;
	if(FATType==FAT32){
		seekreadRW(ThisFATSecNum,ThisFATEntOffset,(void*)readbuf,4);
		return *((DWORD*)readbuf);
	}
	else if(FATType==FAT16){
		seekreadRW(ThisFATSecNum,ThisFATEntOffset,(void*)readbuf,2);
		return *((WORD*)readbuf);
	}
}

DWORD GetClusSector(DWORD N){//Get the sector num of Cluster N
	if(FATType==FAT16 && N==0){//FAT16根目录特殊处理
		return FirstDataSector - RootDirSectors;
	}
	return FirstDataSector + ((N-2)*SecPerClus);
}

DWORD GetTotalClus(DWORD fristClus){//Get the total cluster num
	DWORD count=1;
	DWORD EndClus;
	if(FATType==FAT32){
		EndClus=0x0FFFFFF8;
	}
	else if(FATType==FAT16){
		EndClus=0xFFF8;
	}
	while(GetNextClus(fristClus)<EndClus){
		count++;
	}
	return count;
}

int GetDIRinfo(struct DIR * DIRadd,DWORD ClusNum,DWORD DirNum){//get directory information
	return seekreadRW(GetClusSector(ClusNum),DirNum*32,(void*)DIRadd,32);//0--Done,1--Failed
}

int ReadFile(DWORD fristClus,DWORD offset,BYTE*buf,DWORD count){//read file data function (use frist cluster num)
	DWORD ClusSize=BytsPerSec * SecPerClus;//get the Cluster Size num for compute
	DWORD BufCnt=0;//buffer write count
	while(offset> ClusSize){//find the frist byte in which cluster
		offset-=ClusSize;
		fristClus=GetNextClus(fristClus);//offset to next cluster
	}
	if(offset+count>ClusSize){
		BufCnt=ClusSize-offset;//frist read
		count-=BufCnt;//compute the left bytes num
		if(seekreadRW(GetClusSector(fristClus),offset,(void*)buf,BufCnt)==-1)
			return -1;
		while(count>ClusSize){
			count-=ClusSize;//compute the left bytes num
			fristClus=GetNextClus(fristClus);//offset to next cluster
			if(seekreadRW(GetClusSector(fristClus),0,(void*)(buf+BufCnt),ClusSize)==-1)
				return -1;
			BufCnt+=ClusSize;//compute the bytes have read
		}
		if(count>0){
			fristClus=GetNextClus(fristClus);//offset to next cluster
			return seekreadRW(GetClusSector(fristClus),0,(void*)(buf+BufCnt),count);
		}
	}
	else{
		return seekreadRW(GetClusSector(fristClus),offset,(void*)buf,count);//0--Done,1--Failed
	}
}

DWORD GetRootClus(){//get the root dir cluster num
	return RootClus;
}

DWORD GetClusSize(){//get the Cluster Size
	return BytsPerSec * SecPerClus;
}

