#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
//#include <sys/ioctl.h>


int main(void)

{
	int fd,buf;
	printf("device file open sdjtei\n");

	fd=open("/dev/gpio-test",O_RDWR);
	if(fd<0)
	{
		perror("/dev/gpio-test open error\n");
		return 1;
	}


		printf("\nwrite()\n");
		write(fd,buf,sizeof(buf));

		printf("\nioctl(0)\n");
		ioctl(fd,0,NULL);

		printf("\nioctl(1)\n");
		ioctl(fd,1,NULL);
		return 2;	


	close(fd);
	return 0;

}

