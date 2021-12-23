#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "main.h"
#include "com.h"
#define FILE_MAX_SIZE 300000*1000  //设置300 000k的每个文件的最大size，到300m就新建一个文件开始存储
FILE *fp_IMU = NULL;

int fd_IMUODOCom;
int CalculateValue(unsigned char *RawData);
int FindStart();
void set_file_name(char* fileName);
long get_file_size(char* filename);

int main(int argc, char *argv[])
{
	char fileName[100] = {0};
	
	//根据系统时间存储文件
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime); //获取Unix时间戳
	timeinfo = localtime(&rawtime); //转为时间结构
	sprintf(fileName,"./savedata/%d%d-%d%d%d.txt", timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	fp_IMU = fopen(fileName,"a");
	fd_IMUODOCom = openIMUCom("/dev/ttyUSB0");
	if(fd_IMUODOCom < 0)
	{
		printf("Open IMU com error\n");
		return -2;
	}

	int read_len = -1;
	int flag = 0;//判断是否找到帧头
	unsigned char temp[512];//暂存串口读到的数据
	unsigned char read_buff[44];
    int t_sec_num = 0;
    int last_sec = 0;
	time_t t;
	struct tm *now;
    struct tm *last;
    struct timeval tv;
    float t_usec;
	double Gyr_x,Gyr_y,Gyr_z,Acc_x,Acc_y,Acc_z;
	char ch[500]={0};
	int Sum_read_len = 0;
	sprintf(ch,"t_hour   t_min   t_sec   t_usec   t_sec_num    Gyr_x    Gyr_y	Gyr_z	Acc_x	Acc_y	Acc_z	count\n");
	fputs(ch,fp_IMU);
	fflush(fp_IMU);
	int count_data = 0;
	while(1)
	{
		while(!flag)
		{
			flag = FindStart();
		}
	    t = time(NULL);          //获取当前系统时间 t为从某某时间到现在的秒数
	    now = localtime(&t);     //将t转换成tm类型的结构体
              
	    if(now == NULL)
	     {
		     perror("localtime");
		     return 0;
	     }
        gettimeofday(&tv,NULL);
        t_usec = (float)tv.tv_usec/1000000+now->tm_sec;
        if (now->tm_sec == last_sec)
			t_sec_num = t_sec_num + 1;//用来统计每秒的数据个数
        else 
        {
			t_sec_num = 1;
        }
        last_sec = now->tm_sec ;
      
		memset(temp,0,sizeof(temp));
		while(Sum_read_len < 44)
		{
			read_len = read(fd_IMUODOCom,temp,4);
			memcpy(read_buff+Sum_read_len,temp,4);
			Sum_read_len += read_len;
		}
		//printf("Sum_read_len = %d\n",Sum_read_len);
		Sum_read_len = 0;

		if(read_buff[0] != 0xA5 || read_buff[42] != 0x0D ||read_buff[43] != 0x0A)
			flag = 0;
		else
		{
			count_data++;
			if (count_data==125)
			{
				printf("get 125 pice of IMU data");//获取125条数据打印一次，不然打印太频繁了
				count_data = 0;
			}
			
			Gyr_x = (double)CalculateValue(read_buff+1)/16384;//说明书里的公式
			Gyr_y = (double)CalculateValue(read_buff+4)/16384;
			Gyr_z = (double)CalculateValue(read_buff+7)/16384;
			Acc_x = (double)CalculateValue(read_buff+11)/524288;
			Acc_y = (double)CalculateValue(read_buff+14)/524288;
			Acc_z = (double)CalculateValue(read_buff+17)/524288;

			sprintf(ch,"%d %d %d %.4f %d %.8f %.8f %.8f %.8f %.8f %.8f %x\r\n",now->tm_hour,now->tm_min,now->tm_sec,t_usec,t_sec_num, Gyr_x,Gyr_y,Gyr_z,Acc_x,Acc_y,Acc_z,*(read_buff+35));
            fputs(ch,fp_IMU);
			fflush(fp_IMU);//刷新流 stream 的输出缓冲区。
		}
		long length = get_file_size(fileName);
		if (length >= FILE_MAX_SIZE)
		{
			//创建新的文件
			memset(fileName, 0, sizeof(fileName));
			set_file_name(fileName); 
			fp_IMU = fopen(fileName, "a");
		}
	}

	pause();
	fclose(fp_IMU);
	return 0;
}

int FindStart()
{
	int NICE;
	char b4_read[2];
	while(1)
	{
		if(b4_read[0] != 0x0D)
			read(fd_IMUODOCom,b4_read,1);
		else
		{
			read(fd_IMUODOCom,b4_read+1,1);
			if(b4_read[1] == 0x0A)
				break;
		}
		//b4_read[0] == 0x00;
		//b4_read[1] == 0x00;
	}
	NICE = 1;
	return NICE;
}

long get_file_size(char* filename)
{
	long length = 0;
	FILE* fp = NULL;
	fp = fopen(filename, "r");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
	}
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
	return length;
}

void set_file_name(char* fileName)
{
	//根据系统时间存储文件
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime); //获取Unix时间戳
	timeinfo = localtime(&rawtime); //转为时间结构
	sprintf(fileName, "./savedata/%d%d-%d%d%d", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

int CalculateValue(unsigned char *RawData)
{
	int Value;
	if(RawData[0] <= 0x80)
		Value=RawData[0]*65536+RawData[1]*256+RawData[2];
	else
		Value=RawData[0]*65536+RawData[1]*256+RawData[2]-16777216;
	return Value;
}
