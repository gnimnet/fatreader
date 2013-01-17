#include <stdio.h>
#include <stdlib.h>
/*
*  mytype.h  --  this header file define the types my use
*  rwfun.h  --  this header file has the read & write function declared
*  fat.h  --  this header file has the FAT function and structure declared
*/
#include "mytype.h"
#include "rwfun.h"
#include "fat.h"

#define BUFSIZE 32768

void PrintBPB(struct BPB * BPBadd);//BPB infomation print functions
void PrintFSI(struct FSI * FSIadd);//FSI infomation print functions
void PrintDIR(struct DIR * FSIadd);//DIR infomation print functions
void printDate(WORD date);//print the WORD date
void printTime(WORD time);//print the WORD time
void printFileAttr(BYTE Attr);//print the info of attributes byte
void printhex(BYTE ch);//print a char to hex num
void printbuf(WORD count);//print the test buf
void printDirFiles(DWORD DirFristClus);//print a dir file list
void printShortName(char DIR_Name[]);//print a file short name
int printLongName(DWORD ClusNum,DWORD DirNum);//print a file long name
int printVolID();//print the volume id

int ChangePWD(DWORD * PWDClusAdd,int Offset);//chage pwd function
int CatFile(DWORD * PWDClusAdd,int Offset);//cat a file data

BYTE buf[BUFSIZE];

struct BPB MYBPB;
struct FSI MYFSI;
struct DIR MYDIR;
int Fstype;
DWORD PwdClus;

int main(int argc,char*argv[]){
	char cmd;
	int offset;
	if(argc!=2){//make sure argument num
		printf("usage:%s diskfile\n",argv[0]);
		exit(1);
	}
	if(initRW(argv[1])==-1){//init the disk for read
		printf("open device failed!\n");
		exit(2);
	}
	if(GetBPB(&MYBPB)==-1){//get BPB information
		printf("Read Disk Failed!\n");
		exit(3);
	}
	PrintBPB(&MYBPB);//print BPB information
	switch(Fstype=GetFATType(&MYBPB)){
		case FAT32:printf("File system is FAT32\n");break;
		case FAT16:printf("File system is FAT16\n");break;
		case FAT12:printf("File system is FAT12\n");break;
		default:printf("File system unknow\n");
	}
	if(GetFAT32FSI(*((WORD*)MYBPB.BPB_FSInfo),&MYFSI)==0)//File system is FAT32,and get FSI ok,print it!
		PrintFSI(&MYFSI);
	//***************************Get Root Cluster*************************//
	if(Fstype==FAT32||Fstype==FAT16){
		printf("FAT type support!\n");
	}
	else{
		printf("Not support FAT12...\n");
		exit(6);
	}
	printf("Volume ID:");
	if(printVolID()==-1){//print volume id
		printf("No Volume ID...\n");
	}
	PwdClus=GetRootClus();//Set PWD to Root dir
	printDirFiles(PwdClus);//print files in PWD dir
	printf("----------File list of Root Dir----------\n");
	while(1){
		printf("Cmd:");
		cmd=getchar();
		if(cmd=='c'){//change pwd command
			printf("Offset:");
			scanf("%d",&offset);
			getchar();
			if(ChangePWD(&PwdClus,offset)==-1){
				printf("change pwd failed\n");
			}
			printDirFiles(PwdClus);
		}
		else if(cmd=='s'){//show file command
			printf("Offset:");
			scanf("%d",&offset);
			getchar();
			if(CatFile(&PwdClus,offset)==-1){
				printf("print file failed\n");
			}
			printDirFiles(PwdClus);
		}
		else if(cmd=='q'){//quit command
			printf("program close...\n");
			return 0;
		}
		else{
			printf("unknow command...\n");
		}
	}
	return 0;
}

void PrintBPB(struct BPB * BPBadd){//BPB infomation print functions
	/******************************PART1******************************/
	printf("------------------------------------------------------\n");
	//BS_jmpBoot
	printf("Boot jump instruction:");
	printhex((*BPBadd).BS_jmpBoot[0]);
	printhex((*BPBadd).BS_jmpBoot[1]);
	printhex((*BPBadd).BS_jmpBoot[2]);
	putchar('\n');
	//OEMName
	printf("OEM name:");
	putchar((*BPBadd).OEMName[0]);
	putchar((*BPBadd).OEMName[1]);
	putchar((*BPBadd).OEMName[2]);
	putchar((*BPBadd).OEMName[3]);
	putchar((*BPBadd).OEMName[4]);
	putchar((*BPBadd).OEMName[5]);
	putchar((*BPBadd).OEMName[6]);
	putchar((*BPBadd).OEMName[7]);
	putchar('\n');
	//BPB_BytsPerSec
	printf("Bytes per sector:%d\n",*((WORD*)(*BPBadd).BPB_BytsPerSec));
	//BPB_SecPerClus
	printf("Sector per cluster:%d\n",(*BPBadd).BPB_SecPerClus);
	//BPB_RsvdSecCnt
	printf("Reserved sectors:%d\n",*((WORD*)(*BPBadd).BPB_RsvdSecCnt));
	//BPB_NumFATs
	printf("Num of FATs:%d\n",(*BPBadd).BPB_NumFATs);
	//BPB_RootEntCnt
	printf("Entries in the root directory(FAT16/12):%d\n",*((WORD*)(*BPBadd).BPB_RootEntCnt));
	//BPB_TotSec16
	printf("Count of all sectors(FAT16/12):%d\n",*((WORD*)(*BPBadd).BPB_TotSec16));
	//BPB_Media
	printf("Disk media(0xF8--fixed):0x%X\n",(*BPBadd).BPB_Media);
	// BPB_FATSz16
	printf("Count of sectors occupied by ONE FAT(FAT16/12):%d\n",*((WORD*)(*BPBadd).BPB_FATSz16));
	//BPB_SecPerTrk
	printf("Sector per track for interrupt 0x13:%d\n",*((WORD*)(*BPBadd).BPB_SecPerTrk));
	//BPB_NumHeads
	printf("Number of heads for interrupt 0x13:%d\n",*((WORD*)(*BPBadd).BPB_NumHeads));
	//BPB_HiddSec
	printf("Hidden sectors in this FAT volume:%d\n",*((DWORD*)(*BPBadd).BPB_HiddSec));
	//BPB_TotSec32
	printf("Total count of sectors on the volume(FAT32):%d\n",*((DWORD*)(*BPBadd).BPB_TotSec32));

	/******************************PART2******************************/
	if(*((WORD*)(*BPBadd).BPB_FATSz16)!=0){
		printf("--------------------FAT16 or FAT12--------------------\n");
	}
	else{
		printf("------------------------FAT 32------------------------\n");
		//BPB_FATSz32
		printf("Count of sectors occupied by ONE FAT:%d\n",*((DWORD*)(*BPBadd).BPB_FATSz32));
		//BPB_ExtFlags
		printf("Extend flags:");
		printhex((*BPBadd).BPB_ExtFlags[0]);
		printhex((*BPBadd).BPB_ExtFlags[1]);
		putchar('\n');
		//BPB_FSVer
		printf("FAT32 Version:");
		printhex((*BPBadd).BPB_FSVer[0]);
		printhex((*BPBadd).BPB_FSVer[1]);
		putchar('\n');
		//BPB_RootClus
		printf("Cluster number of the first cluster of the root directory:%d\n",*((DWORD*)(*BPBadd).BPB_RootClus));
		//BPB_FSInfo
		printf("Sector number of FSINFO structure in reserved area:%d\n",*((WORD*)(*BPBadd).BPB_FSInfo));
		//BPB_BkBootSec
		printf("Sector number in reserved area of a copy of the boot record:%d\n",*((WORD*)(*BPBadd).BPB_BkBootSec));
		//BPB_Reserved
		printf("Reserved:");
		printhex((*BPBadd).BPB_Reserved[0]);
		printhex((*BPBadd).BPB_Reserved[1]);
		printhex((*BPBadd).BPB_Reserved[2]);
		printhex((*BPBadd).BPB_Reserved[3]);
		printhex((*BPBadd).BPB_Reserved[4]);
		printhex((*BPBadd).BPB_Reserved[5]);
		printhex((*BPBadd).BPB_Reserved[6]);
		printhex((*BPBadd).BPB_Reserved[7]);
		printhex((*BPBadd).BPB_Reserved[8]);
		printhex((*BPBadd).BPB_Reserved[9]);
		printhex((*BPBadd).BPB_Reserved[10]);
		putchar('\n');
	}
	//BS_DrvNum
	printf("Int 0x13 drive number:%d\n",(*BPBadd).BS_DrvNum);
	//BS_Reserved1
	printf("Reserved (used by Windows NT):0x%X\n",(*BPBadd).BS_Reserved1);
	//BS_BootSig
	printf("Extended boot signature (0x29):0x%X\n",(*BPBadd).BS_BootSig);
	//BS_VolID
	printf("Volume serial number:%u\n",*((DWORD*)(*BPBadd).BS_VolID));
	//BS_VolLab
	printf("Volume Label:");
	putchar((*BPBadd).BS_VolLab[0]);
	putchar((*BPBadd).BS_VolLab[1]);
	putchar((*BPBadd).BS_VolLab[2]);
	putchar((*BPBadd).BS_VolLab[3]);
	putchar((*BPBadd).BS_VolLab[4]);
	putchar((*BPBadd).BS_VolLab[5]);
	putchar((*BPBadd).BS_VolLab[6]);
	putchar((*BPBadd).BS_VolLab[7]);
	putchar((*BPBadd).BS_VolLab[8]);
	putchar((*BPBadd).BS_VolLab[9]);
	putchar((*BPBadd).BS_VolLab[10]);
	putchar('\n');
	//BS_FilSysType
	printf("File System Type(just a string!):");
	putchar((*BPBadd).BS_FilSysType[0]);
	putchar((*BPBadd).BS_FilSysType[1]);
	putchar((*BPBadd).BS_FilSysType[2]);
	putchar((*BPBadd).BS_FilSysType[3]);
	putchar((*BPBadd).BS_FilSysType[4]);
	putchar((*BPBadd).BS_FilSysType[5]);
	putchar((*BPBadd).BS_FilSysType[6]);
	putchar((*BPBadd).BS_FilSysType[7]);
	putchar('\n');
}

void PrintFSI(struct FSI * FSIadd){//FSInfo print functions
	printf("--------------------FSInfo--------------------\n");
	//FSI_LeadSig
	printf("FSI_LeadSig:");
	printhex((*FSIadd).FSI_LeadSig[0]);
	printhex((*FSIadd).FSI_LeadSig[1]);
	printhex((*FSIadd).FSI_LeadSig[2]);
	printhex((*FSIadd).FSI_LeadSig[3]);
	putchar('\n');
	//FSI_StrucSig
	printf("FSI_StrucSig:");
	printhex((*FSIadd).FSI_StrucSig[0]);
	printhex((*FSIadd).FSI_StrucSig[1]);
	printhex((*FSIadd).FSI_StrucSig[2]);
	printhex((*FSIadd).FSI_StrucSig[3]);
	putchar('\n');
	//FSI_Free_Count
	printf("Free cluster count:0x%X\n",*((DWORD*)(*FSIadd).FSI_Free_Count));
	//FSI_Nxt_Free
	printf("Address start looking for free clusters:0x%X\n",*((DWORD*)(*FSIadd).FSI_Nxt_Free));
	//FSI_TrailSig
	printf("FSI_TrailSig:");
	printhex((*FSIadd).FSI_TrailSig[0]);
	printhex((*FSIadd).FSI_TrailSig[1]);
	printhex((*FSIadd).FSI_TrailSig[2]);
	printhex((*FSIadd).FSI_TrailSig[3]);
	putchar('\n');
}

void PrintDIR(struct DIR * DIRadd){//DIR infomation print functions
	printf("--------------------File information--------------------\n");
	//DIR_Name
	printf("Directory Name(short name):");
	putchar((*DIRadd).DIR_Name[0]);
	putchar((*DIRadd).DIR_Name[1]);
	putchar((*DIRadd).DIR_Name[2]);
	putchar((*DIRadd).DIR_Name[3]);
	putchar((*DIRadd).DIR_Name[4]);
	putchar((*DIRadd).DIR_Name[5]);
	putchar((*DIRadd).DIR_Name[6]);
	putchar((*DIRadd).DIR_Name[7]);
	putchar((*DIRadd).DIR_Name[8]);
	putchar((*DIRadd).DIR_Name[9]);
	putchar((*DIRadd).DIR_Name[10]);
	putchar('\n');
	//DIR_Attr
	printf("File attributes:0x");
	printhex((*DIRadd).DIR_Attr);
	putchar('\n');
	printf("-------------------\n");
	printFileAttr((*DIRadd).DIR_Attr);
	printf("-------------------\n");
	//DIR_NTRes
	printf("Reserved for use by Windows NT:0x");
	printhex((*DIRadd).DIR_NTRes);
	putchar('\n');
	//DIR_CrtTimeTenth
	printf("Create time millisecond stamp:%f\n",((float)(*DIRadd).DIR_CrtTimeTenth)/100);
	//DIR_CrtTime
	printf("Create time:");
	printTime(*((WORD*)(*DIRadd).DIR_CrtTime));
	//DIR_CrtDate
	printf("Create date:");
	printDate(*((WORD*)(*DIRadd).DIR_CrtDate));
	//DIR_LstAccDate
	printf("Last access date:");
	printDate(*((WORD*)(*DIRadd).DIR_LstAccDate));
	//DIR_FstClusHI
	printf("Cluster High:");
	printhex((*DIRadd).DIR_FstClusHI[0]);
	printhex((*DIRadd).DIR_FstClusHI[1]);
	putchar('\n');
	//DIR_WrtTime
	printf("Write time:");
	printTime(*((WORD*)(*DIRadd).DIR_WrtTime));
	//DIR_WrtDate
	printf("Write date:");
	printDate(*((WORD*)(*DIRadd).DIR_WrtDate));
	//DIR_FstClusLO
	printf("Cluster Low:");
	printhex((*DIRadd).DIR_FstClusLO[0]);
	printhex((*DIRadd).DIR_FstClusLO[1]);
	putchar('\n');
	//DIR_FileSize
	printf("File Size(byte):%u\n",*((DWORD*)(*DIRadd).DIR_FileSize));
}

void printDate(WORD date){
	printf("%d-%d-%d\n",
		((date & 0xFE00)>>9)+1980,
		(date & 0x01E0)>>5,
		(date & 0x001F));
}

void printTime(WORD time){
	printf("%d:%d:%d\n",
		(time & 0xF800)>>11,
		(time & 0x07E0)>>5,
		(time & 0x001F)<<1);
}

void printbuf(WORD count){
	WORD line=0;
	int i;
	printf("--------------------BUFFER--------------------\n");
	while(count!=0){
		printhex(line/256);
		printhex(line%256);
		putchar(':');
		if(count>=16){
			for(i=0;i<16;i++)
				printhex(buf[line*16+i]);
			putchar('|');
			for(i=0;i<16;i++)
				putchar(buf[line*16+i]);
			putchar('\n');
			count-=16;
			line++;
		}
		else{
			for(i=0;i<count;i++)
				printhex(buf[line*16+i]);
			putchar('|');
			for(i=0;i<count;i++)
				putchar(buf[line*16+i]);
			putchar('\n');
			count-=16;
			count=0;
		}
	}
	printf("--------------------BUFEND--------------------\n");
}

void printFileAttr(BYTE Attr){
	if(Attr & ATTR_READ_ONLY)
		printf("ATTR_READ_ONLY\n");
	if(Attr & ATTR_HIDDEN)
		printf("ATTR_HIDDEN\n");
	if(Attr & ATTR_SYSTEM)
		printf("ATTR_SYSTEM\n");
	if(Attr & ATTR_VOLUME_ID)
		printf("ATTR_VOLUME_ID\n");
	if(Attr & ATTR_DIRECTORY)
		printf("ATTR_DIRECTORY\n");
	if(Attr & ATTR_ARCHIVE)
		printf("ATTR_ARCHIVE\n");
	if(Attr & ATTR_LONG_NAME)
		printf("ATTR_LONG_NAME\n");
}

void printDirFiles(DWORD DirFristClus){
	int cnt=GetTotalClus(DirFristClus);
	int totalnum=GetClusSize()/32;
	int i;
	int filecount=0;
	struct DIR DIRinfo;
	printf("File Name\tType\tSize\tCluster Num\tOffset\n");
	while((cnt--)>=1){
		for(i=0;i<totalnum;i++){
			if(GetDIRinfo(&DIRinfo,DirFristClus,i)==-1){//get file information
				printf("Read Disk Failed!\n");
				exit(999);
			}
			if(DIRinfo.DIR_Name[0]==0xE5)//Empty
				continue;
			if(DIRinfo.DIR_Name[0]==0x00)//End
				break;
			if(DIRinfo.DIR_Attr==ATTR_LONG_NAME)//Long File Name
				continue;
			printShortName(DIRinfo.DIR_Name);
			if(DIRinfo.DIR_Attr==ATTR_VOLUME_ID)
				printf("VOL ID");
			else if(DIRinfo.DIR_Attr & ATTR_DIRECTORY)
				printf("DIR");
			else
				printf("FILE");
			printf("\t%d\t",*((DWORD*)DIRinfo.DIR_FileSize));
			printhex(DIRinfo.DIR_FstClusHI[1]);
			printhex(DIRinfo.DIR_FstClusHI[0]);
			printhex(DIRinfo.DIR_FstClusLO[1]);
			printhex(DIRinfo.DIR_FstClusLO[0]);
			printf("\t%d",i);
			if(i==0){
				putchar('\n');
				continue;
			}
			printf("\nLong file name:");
			if(printLongName(DirFristClus,i)==-1)
				printf("[No Long File Name ...]");
			putchar('\n');
			
			filecount++;
		}
		DirFristClus=GetNextClus(DirFristClus);
	}
	printf("Total file num:%d\n",filecount);
}

void printShortName(char DIR_Name[]){
	int i,j,k;
	for(i=7;i>=0;i--){
		if(DIR_Name[i]!=0x20)
			break;
	}
	for(j=10;j>=8;j--){
		if(DIR_Name[j]!=0x20)
			break;
	}
	if(DIR_Name[0]==0x05)//instead 0x05 for 0xE5 in the frist char
		putchar(0xE5);
	else
		putchar(DIR_Name[0]);
	for(k=1;k<=i;k++)
		putchar(DIR_Name[k]);
	if(j!=7){
		putchar('.');
		for(k=8;k<=j;k++)
			putchar(DIR_Name[k]);
	}
	if(j!=7)
		j-=6;
	else
		j-=7;
	if(i+j+1<8)
		putchar('\t');
	putchar('\t');
}

int printLongName(DWORD ClusNum,DWORD DirNum){
	struct LDIR LDIRinfo;
	int i;
	int cnt=0;
	while(1){
		if(GetDIRinfo((struct DIR *)&LDIRinfo,ClusNum,--DirNum)==-1){//get file information
			printf("Read Disk Failed!\n");
			exit(999);
		}
		cnt++;//for the DIRinfo num have read
		if(LDIRinfo.LDIR_Attr!=ATTR_LONG_NAME){//not a long name LDIR
			if(cnt==1)
				return -1;
		}
		if(LDIRinfo.LDIR_Ord==0xE5){//End(this case may be some error...)
			if(cnt==1)
				return -1;
		}
		for(i=0;i<10;i++){
			if(i%2==0 && *((WORD*)&LDIRinfo.LDIR_Name1[i])==0x0000)//long name over
				return 0;
			putchar(LDIRinfo.LDIR_Name1[i]);
		}
		for(i=0;i<12;i++){
			if(i%2==0 && *((WORD*)&LDIRinfo.LDIR_Name2[i])==0x0000)//long name over
				return 0;
			putchar(LDIRinfo.LDIR_Name2[i]);
		}
		for(i=0;i<4;i++){
			if(i%2==0 && *((WORD*)&LDIRinfo.LDIR_Name3[i])==0x0000)//long name over
				return 0;
			putchar(LDIRinfo.LDIR_Name3[i]);
		}
		if(LDIRinfo.LDIR_Ord & LAST_LONG_ENTRY)//end of long file name LDIR
			return 0;
	}
}

void printhex(BYTE ch){//function use to print a char to hex data
	if(ch>0x0F){
		printf(" %X",ch);
	}
	else{
		printf(" 0%X",ch);
	}
}

int ChangePWD(DWORD * PWDClusAdd,int Offset){
	struct DIR DIRinfo;
	if(GetDIRinfo(&DIRinfo,*PWDClusAdd,Offset)==-1){//get file information
		printf("Read Disk Failed!\n");
		exit(999);
	}
	if(DIRinfo.DIR_Attr==ATTR_LONG_NAME || DIRinfo.DIR_Attr!=ATTR_DIRECTORY){
		printShortName(DIRinfo.DIR_Name);
		printf(" is not a dictory!\n");
		return -1;
	}
	else{
		printShortName(DIRinfo.DIR_Name);
		printf(" is now PWD!\n");
		*PWDClusAdd=*((WORD*)&DIRinfo.DIR_FstClusHI);
		*PWDClusAdd=*PWDClusAdd<<16;
		*PWDClusAdd+=*((WORD*)&DIRinfo.DIR_FstClusLO);
		if(*PWDClusAdd==0){//Root dir
			*PWDClusAdd=GetRootClus();
		}
		return 0;
	}
}

int CatFile(DWORD * PWDClusAdd,int Offset){
	struct DIR DIRinfo;
	DWORD FileClus;
	if(GetDIRinfo(&DIRinfo,*PWDClusAdd,Offset)==-1){//get file information
		printf("Read Disk Failed!\n");
		exit(999);
	}
	if(DIRinfo.DIR_Attr==ATTR_VOLUME_ID || DIRinfo.DIR_Attr==ATTR_DIRECTORY){
		printShortName(DIRinfo.DIR_Name);
		printf(" is not a file!\n");
		return -1;
	}
	else{
		FileClus=*((WORD*)&DIRinfo.DIR_FstClusHI);
		FileClus=FileClus<<16;
		FileClus+=*((WORD*)&DIRinfo.DIR_FstClusLO);
		if(*((DWORD*)DIRinfo.DIR_FileSize)>BUFSIZE){
			printf("Buffer full!\n");
			return -1;
		}
		if(ReadFile(FileClus,0x00,buf,*((DWORD*)DIRinfo.DIR_FileSize))==-1){
			printf("Read Disk Failed!\n");
			exit(5);
		}
		printbuf(*((DWORD*)DIRinfo.DIR_FileSize));
		printf("Total print char num:%d\n",*((DWORD*)DIRinfo.DIR_FileSize));
		return 0;
	}
}

int printVolID(){//print the volume id
	int cnt=GetTotalClus(GetRootClus());
	int totalnum=GetClusSize()/32;
	int offset;
	int notfind=-1;
	DWORD ClusTmp=GetRootClus();
	struct DIR DIRinfo;
	int i,j,k;
	while((cnt--)>=1){
		for(offset=0;offset<totalnum;offset++){
			if(GetDIRinfo(&DIRinfo,ClusTmp,offset)==-1){//get file information
				printf("Read Disk Failed!\n");
				exit(999);
			}
			if(DIRinfo.DIR_Name[0]==0x00)//End
				break;
			if(DIRinfo.DIR_Attr==ATTR_VOLUME_ID){//Volume ID
				for(i=7;i>=0;i--){
					if(DIRinfo.DIR_Name[i]!=0x20)
						break;
				}
				for(j=10;j>=8;j--){
					if(DIRinfo.DIR_Name[j]!=0x20)
						break;
				}
				if(DIRinfo.DIR_Name[0]==0x05)//instead 0x05 for 0xE5 in the frist char
					putchar(0xE5);
				else
					putchar(DIRinfo.DIR_Name[0]);
				for(k=1;k<=i;k++)
					putchar(DIRinfo.DIR_Name[k]);
				if(j!=7){
					for(k=8;k<=j;k++)
						putchar(DIRinfo.DIR_Name[k]);
				}
				//printShortName(DIRinfo.DIR_Name);
				putchar('\n');
				notfind=0;
				break;
			}
		}
		ClusTmp=GetNextClus(ClusTmp);
	}
	return notfind;
}
