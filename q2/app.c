#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include <sys/stat.h>

#define WR_VALUE _IOW('a','a',int32_t)
#define RD_VALUE _IOR('a','b',int32_t)

int main(int argc,char const *argv[])
{
    int fd;
    char option;
    int32_t value,number;
    printf("\n********opening driver*******\n");

    if((fd=open("/dev/ioctl_device",O_RDWR))<0){
        perror("Error in opening device file\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("PLEASE CHOOSE YOUR OPTION:\n");
      
        printf("1. WRITE\n");
        printf("2. READ\n");
        printf("3.EXIT\n");
        scanf("%c",&option);
        printf("your option = %c\n",option);

        switch(option)
        {
            case '1':
                printf("enter value to send : ");
                scanf("%d",&value);
                printf("writing value to driver...");
                ioctl(fd,WR_VALUE,(int32_t*)&value);
                printf("done\n");
                break;
            case '2':
                  
                printf("reading value from the driver...");
                ioctl(fd,RD_VALUE,(int32_t*)&number);
                printf("done\n");
                break;
            
            case '3':
                printf("CLOSING DRIVER\n");
                exit(1);
                break;
            default:
                printf("enter valid option = %c\n",option);
                break;
        }
    }
    close(fd);
    return 0;
    
}