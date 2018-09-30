#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include"io.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

#define BYTESOFSECTOR 512

int AdjustInSectorSize(int iFd,int iSourceSize);
void WriteKernelInformation(int iTargetFd, int iKernelSectorCount);
int CopyFile(int iSourceFd, int iTargetFd);
void CalculateHash(const char* fileName,int iTargetFd,int iSourceFileSize);

int main(int argc, char* argv[])
{
	int iSourceFd;
	int iTargetFd;
	int iBootLoaderSize;
	int iKernel32SectorCount;
	int iSourceSize;
	int lPosition;

	if(argc<3)
	{
		fprintf(stderr,"[ERROR] ImageMaker.exe BootLoader.bin Kernel32.bin\n");
		exit(-1);
	}

	if((iTargetFd = open("Disk.img",O_RDWR | O_CREAT | O_TRUNC | S_IREAD | S_IWRITE)) == -1)
	{
		fprintf(stderr, "[ERROR] Disk.img open fail.\n");
		exit(-1);
	}

	printf("[INFO] Copy boot loader to image file\n");
	if((iSourceFd = open(argv[1], O_RDONLY )) == -1)
	{
		fprintf(stderr,"[ERROR] %s open fail\n",argv[1]);
		exit(-1);
	}


	iSourceSize = CopyFile(iSourceFd,iTargetFd);
	close(iSourceFd);

	iBootLoaderSize = AdjustInSectorSize(iTargetFd, iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n",argv[1],iSourceSize,iBootLoaderSize);

	printf("[INFO] Copy protected mode kernel to image file\n");

	if((iSourceFd = open(argv[2],O_RDONLY)) == -1)
	{
		fprintf(stderr,"[ERROR] %s open fail\n",argv[2]);
		exit(-1);
	}

	iSourceSize = CopyFile(iSourceFd,iTargetFd);
	close(iSourceFd);

	iKernel32SectorCount = AdjustInSectorSize(iTargetFd,iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n",argv[2],iSourceSize,iKernel32SectorCount);

	
	printf("\n***********************************************\n");
	CalculateHash(argv[2],iTargetFd,iSourceSize);
	lPosition = lseek(iTargetFd,(off_t)512,SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "[ERROR] lseek fail.");
		exit(-1);
	}
	if((iSourceFd = open(argv[2],O_RDONLY)) == -1)
	{
		fprintf(stderr,"[ERROR] %s open fail\n",argv[2]);
		exit(-1);
	}
	iSourceSize = CopyFile(iSourceFd,iTargetFd);
	close(iSourceFd);

	iKernel32SectorCount = AdjustInSectorSize(iTargetFd,iSourceSize);
	printf("[INFO] %s size = [%d] and sector count = [%d]\n",argv[2],iSourceSize,iKernel32SectorCount);

	printf("\n*************************************************\n");
	
	printf("[INFO] Start to write kernel information\n");
	WriteKernelInformation(iTargetFd, iKernel32SectorCount);
	printf("[INFO] Image file create complete\n");
	
	close(iTargetFd);
	return 0;
}

int AdjustInSectorSize(int iFd, int iSourceSize)
{
	int i;
	int iAdjustSizeToSector;
	char cCh;
	int iSectorCount;

	iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
	cCh = 0x00;

	if(iAdjustSizeToSector != 0)
	{
		iAdjustSizeToSector = 512 - iAdjustSizeToSector;
		printf("[INFO] File size [%lu] and fill [%u] byte\n",iSourceSize,iAdjustSizeToSector);
		for(i=0;i<iAdjustSizeToSector;++i)
		{
			write(iFd,&cCh,1);
		}
	}

	else{
		printf("[INFO] File size is aligned 512 byte\n");
	}

	iSectorCount = (iSourceSize + iAdjustSizeToSector) / BYTESOFSECTOR;
	return iSectorCount;
}

void WriteKernelInformation(int iTargetFd, int iKernelSectorCount)
{
	unsigned short usData;
	long lPosition;

	lPosition = lseek(iTargetFd, (off_t)5,SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "lseek fail. Return value = %d, errno = %d,  %d\n",lPosition,errno,SEEK_SET);
		exit(-1);
	}

	usData = (unsigned short)iKernelSectorCount;
	write(iTargetFd, &usData,2);

	printf("[INFO] Total sector count except boot loader [%d]\n",iKernelSectorCount);
}

int CopyFile(int iSourceFd, int iTargetFd)
{
	int iSourceFileSize;
	int iRead;
	int iWrite;
	char vcBuffer[BYTESOFSECTOR];

	iSourceFileSize = 0;
	while(1)
	{
		iRead = read(iSourceFd,vcBuffer,sizeof(vcBuffer));
		iWrite = write(iTargetFd, vcBuffer, iRead);

		if(iRead != iWrite)
		{
			fprintf(stderr, "[ERROR] iRead != iWrite..\n");
			exit(-1);
		}

		iSourceFileSize += iRead;

		if(iRead != sizeof(vcBuffer))
		{
			break;
		}
	}
	printf("\niSourceFileSize : %d\n",iSourceFileSize);
	return iSourceFileSize;
}

void CalculateHash(const char* fileName,int iTargetFd, int iSourceSize){
	char byteHash[4] = {'\0',};
	int iRead;
	int iWrite;
	char vcBuffer[4];
	long lPosition;
	unsigned short usData;
	int iSourceFd;
	int count=0;
	char tpBuffer[iSourceSize];

	
	lPosition = lseek(iTargetFd,(off_t)512,SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "[ERROR] lseek fail.");
		exit(-1);
	}
	while(1){
		
		iRead = read(iTargetFd,vcBuffer,sizeof(vcBuffer));

		if(count==0)
			for(int i=0;i<sizeof(vcBuffer);++i)
				byteHash[i] = vcBuffer[i];
		else
		for(int i=0;i<sizeof(vcBuffer);++i)
			byteHash[i] ^= vcBuffer[i];

		if(iRead!=sizeof(vcBuffer))
			break;
		
		count++;
	}


	if((iSourceFd = open(fileName,O_RDWR)) == -1)
	{
		fprintf(stderr,"[ERROR] %s open fail\n",fileName);
		exit(-1);
	}
		iRead = read(iSourceFd,tpBuffer,sizeof(tpBuffer));

	lPosition = lseek(iSourceFd,(off_t)4,SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "[ERROR] lseek fail.");
		exit(-1);
	}

	write(iSourceFd,tpBuffer,sizeof(tpBuffer));
	printf("Hash value : ");
	
	lPosition = lseek(iSourceFd,(off_t)0,SEEK_SET);
	if(lPosition == -1)
	{
		fprintf(stderr, "[ERROR] lseek fail.");
		exit(-1);
	}
	
	for(int i=0;i<sizeof(byteHash);++i){
	//	usData = (unsigned short)byteHash[i];
		write(iSourceFd,&byteHash[i],1);
		printf("%x ",byteHash[i]);
	}


	close(iSourceFd);
	printf("\n");
}


	

	






