/*
*	This file has FAT struct and function declared
*	By Ming	2009-1
*/
#ifndef _FAT_H_
#define _FAT_H_

#include "rwfun.h"
#include "mytype.h"

//These defines for file system type
#define FAT32 0
#define FAT16 1
#define FAT12 2

//These defines for file attributes byte
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID)

//for LDIR_Ord Mark byte
#define LAST_LONG_ENTRY 0x40

//Define BPB (BIOS Parameter Block) Structure
struct BPB{
	/********************frist 36 Bytes in BPB(same for FAT32 & FAT16)********************/
	BYTE BS_jmpBoot[3];//jump instruction
	BYTE OEMName[8];//OEM name
	BYTE BPB_BytsPerSec[2];//bytes per sector
	BYTE BPB_SecPerClus;//sector per cluster
	BYTE BPB_RsvdSecCnt[2];//reserved sectors nums
	BYTE BPB_NumFATs;//the count of FAT data structures
	BYTE BPB_RootEntCnt[2];//entries in the root directory(FAT16/12)
	BYTE BPB_TotSec16[2];//count of all sectors(FAT16/12)
	BYTE BPB_Media;//0xF8 is the standard value for non-removable media
	BYTE BPB_FATSz16[2];//count of sectors occupied by ONE FAT
	BYTE BPB_SecPerTrk[2];//sector per track for interrupt 0x13
	BYTE BPB_NumHeads[2];//number of heads for interrupt 0x13
	BYTE BPB_HiddSec[4];//count of hidden sectors in this FAT volume
	BYTE BPB_TotSec32[4];//32-bit total count of sectors on the volume
	/********************After 36 byte Infomation(54 byte total)********************/
	//frist 28 bytes,information just for FAT32
	BYTE BPB_FATSz32[4];//32-bit count of sectors occupied by ONE FAT
	BYTE BPB_ExtFlags[2];//Bits 0-3 -- Zero-based number of active FAT. Only valid if mirroring is disabled.
				//Bit 7 -- 0 means the FAT is mirrored at runtime into all FATs.
				//	  -- 1 means only one FAT is active; it is the one referenced in bits 0-3.
	BYTE BPB_FSVer[2];//FAT32 version
	BYTE BPB_RootClus[4];//cluster number of the first cluster of the root directory
	BYTE BPB_FSInfo[2];//Sector number of FSINFO structure in the reserved area of the FAT32 volume
	BYTE BPB_BkBootSec[2];//If non-zero, indicates the sector number in the reserved area of the volume of a copy of the boot record
	BYTE BPB_Reserved[12];//Reserved,fill 0
	//down 26 bytes,information both use for FAT32/16/12
	BYTE BS_DrvNum;//Int 0x13 drive number(0x00 for floppy disks, 0x80 for hard disks)
	BYTE BS_Reserved1;//Reserved (used by Windows NT),to be 0
	BYTE BS_BootSig;//Extended boot signature (0x29)
	BYTE BS_VolID[4];//Volume serial number
	BYTE BS_VolLab[11];//Volume label
	BYTE BS_FilSysType[8];//File System Type String(just a string!!!)
};

//Define FSInfo Sector Structure
struct FSI{
	BYTE FSI_LeadSig[4];//Value 0x41615252
	BYTE FSI_StrucSig[4];//Value 0x61417272
	BYTE FSI_Free_Count[4];//the last known free cluster count on the volume,0xFFFFFFFF means unknown
	BYTE FSI_Nxt_Free[4];//the cluster number at which the driver should start looking for free clusters,0xFFFFFFFF means unknown
	BYTE FSI_TrailSig[4];//Value 0xAA550000
};

//Define Directory Entry Structure
struct DIR{
	BYTE DIR_Name[11];//file name
	BYTE DIR_Attr;//attribute byte
	BYTE DIR_NTRes;//Reserved for use by Windows NT
	BYTE DIR_CrtTimeTenth;//Millisecond stamp at file creation time
	BYTE DIR_CrtTime[2];//Time file was created
	BYTE DIR_CrtDate[2];//Date file was created
	BYTE DIR_LstAccDate[2];//Last access date
	BYTE DIR_FstClusHI[2];//High word of this entry’s first cluster number(FAT32)
	BYTE DIR_WrtTime[2];//Time of last write
	BYTE DIR_WrtDate[2];//Date of last write
	BYTE DIR_FstClusLO[2];//Low word of this entry’s first cluster number
	BYTE DIR_FileSize[4];//32-bit DWORD holding this file’s size in bytes
};

//Define Long Directory Entry Structure
struct LDIR{
	BYTE LDIR_Ord;//The order of this entry,masked with 0x40 -- (LAST_LONG_ENTRY)
	BYTE LDIR_Name1[10];//Characters 1-5 of the long-name
	BYTE LDIR_Attr;//Attributes - must be ATTR_LONG_NAME
	BYTE LDIR_Type;//If zero, indicates a directory entry
	BYTE LDIR_Chksum;//Checksum of name in the short dir entry
	BYTE LDIR_Name2[12];//Characters 6-11 of the long-name
	BYTE LDIR_FstClusLO[2];//Must be ZERO
	BYTE LDIR_Name3[4];//Characters 12-13 of the long-name
};

int GetBPB(struct BPB * BPBadd);//Get BPB infomation
BYTE GetFATType(struct BPB * BPBadd);//Get FAT type and init some information (this function may be call!!!)also return type
int GetFAT32FSI(WORD FSInfo,struct FSI * FSIadd);//Get FAT32 FSInfo,FSInfo may get from BPB(FAT32 only)
DWORD GetNextClus(DWORD N);//Get Next Cluster Number
DWORD GetClusSector(DWORD N);//Get the sector num of Cluster N
DWORD GetTotalClus(DWORD fristClus);//Get the total cluster num
int GetDIRinfo(struct DIR * DIRadd,DWORD ClusNum,DWORD DirNum);//get directory information (also for LDIR)
DWORD GetRootClus();//get the root dir cluster num
DWORD GetClusSize();//get the Cluster Size

//need to know frist cluster,and you must make sure offset+count<=filesize,then use the function
int ReadFile(DWORD fristClus,DWORD offset,BYTE*buf,DWORD count);//read file function

#endif//end of fat.h
