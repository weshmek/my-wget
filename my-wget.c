#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

#define WEB_PORT "80"
#define MY_ERRSTR "Error. Line: %d Error: %s\n"
int main(int argc, char* argv[])
{

	int sock, ret, len, file;
	uint32_t bytes_transmitted;
	struct addrinfo hints;
	struct addrinfo *ai;
	struct addrinfo *ai_beginning;
	struct timeval tv;

	fd_set fds;
	
	char msg[512];
	char repl[512];

	/*Initialize variables*/
	{
		
		memset(&hints, 0, sizeof(hints));
		memset(&tv, 0, sizeof(tv));
		FD_ZERO(&fds);
		bytes_transmitted = 0;
	}



	/*Process Command-line arguments*/
	{	
		if (argc != 4 )

		{	
			printf("Usage: my-wget host file out-file\n");
			exit(0);
		}
	}

	

	
	hints.ai_family = AF_INET;	
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	/*Wait up to 1 second. */
	tv.tv_sec = 1;
	
	ret = getaddrinfo(argv[1], WEB_PORT, &hints, &ai);
	if (ret != 0)
	{
		printf(MY_ERRSTR, __LINE__, gai_strerror(ret));
		exit(ret);
	}

	ai_beginning = ai;

	while(ai != NULL)
	{
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock == -1)
		{
			ai = ai->ai_next;
			continue;
		}
		ret = connect(sock, ai->ai_addr, ai->ai_addrlen);
		if (ret == 0)
		{
			break; /*success*/
		}
		
		ai = ai->ai_next;
		close(sock);
	}

	if (ai == NULL)
	{
		printf(MY_ERRSTR, __LINE__, "ai is NULL");
		exit(errno);
	}
	
	freeaddrinfo(ai_beginning);

	len = sprintf(msg, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", argv[2], argv[1]);
	if (len > sizeof(msg))
	{
		printf(MY_ERRSTR, __LINE__, "Message too long!");
		exit(0);
	}

	ret = write(sock, msg, len);
	if (ret == -1)
	{
		printf(MY_ERRSTR, __LINE__, strerror(errno));
		exit(errno);
	}


	file = open(argv[3], O_WRONLY | O_CREAT);
	if (file == -1)
	{	
		printf(MY_ERRSTR, __LINE__, strerror(errno));
		exit(errno);
	}
	do
	{	
		FD_SET(sock, &fds);
		ret = select(sock + 1, &fds, NULL, NULL, &tv);
		if (ret == 1)
		{
			len = read(sock, repl, sizeof(repl));
			if (len == -1)
			{
				printf(MY_ERRSTR, __LINE__, strerror(errno));
				exit(errno);
			}
			bytes_transmitted += len;
		}
		else if (ret == -1)
		{
			printf(MY_ERRSTR, __LINE__, strerror(errno));
			exit(errno);
		}
		else
		{
			printf("File transferred or timeout occured\n");
			break;
		}
		ret = write(file, repl, len);
		if (ret == -1)
		{
			printf(MY_ERRSTR, __LINE__, strerror(errno));
			exit(errno);
		}

	}while(1);

	ret = close(sock);
	if (ret == -1)
	{
		printf(MY_ERRSTR, __LINE__, strerror(errno));
		exit(errno);
	}


	ret = close(file);
	if (ret == -1)
	{
		printf(MY_ERRSTR, __LINE__, strerror(errno));
		exit(errno);
	}

	printf("%u bytes transmitted\n", bytes_transmitted);

	return 0;
}
