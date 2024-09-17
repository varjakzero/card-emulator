#include <fcntl.h>
#include <linux/serial.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/**
This code is required to create the thread
 */

    // rs422Mode set to true which ultimately makes thread
 	int rs422Mode = 1;
    // defines thread i think
    pthread_t rs422ThreadID;
    // fairly simple int to make sure the thread keeps running  (if running = 1 (do while))
	int running = 1;

 	if (rs422Mode)
	{
        // defines a struct called RS422ThreadArguments
		RS422ThreadArguments arguments = {0};
        // defines things inside struct (forgot term)
		arguments.fd = serialIO;
		arguments.running = &running;
        // creates the thread and parses data into it
		pthread_create(&rs422ThreadID, NULL, rs422Thread, &arguments);
	}

// defines data type of variables inside struct
    typedef struct
{
	int fd;
	int *running;
} RS422ThreadArguments;


/**
 * This thread deals with the RS422 ring network communication. 
 *
 * >This has been isolated from the rest of the code.<
 *
 * Derby Owners Club uses a conversion board that speaks RS422 and
 * converts to RS232. This thread will act as that conversion board
 * and fill the input/output buffer with correct packets.
 **/

 // Opens a void w/ pointer called 'rs422Thread (this was defined earlier). 
 // This in itself contains a void pointer called vargp. 
 // Chatgpt tells me this is apparently a typical way to make a thread
 // void *vargp is a pointer to the threads arguments, this returns a void pointer
 // this can be used to return data, presumeably the converted card data
void *rs422Thread(void *vargp)
{
    //idk
    //defines var (?) RS422ThreadArguments with a pointer called arguments
    //arguments is a void (?) pointer containing vargp (whatever that is)
	RS422ThreadArguments *arguments = (RS422ThreadArguments *)vargp;

	rs422InputBuffer.head = rs422OutputBuffer.head = 0;
	rs422InputBuffer.tail = rs422OutputBuffer.tail = 0;

	while (arguments->running)
	{
		unsigned char buffer[2];

		int bytesLeft = 2;

		int bytesRead = 0;

		while (bytesLeft > 0)
		{
			bytesRead = readBytes(buffer + (2 - bytesLeft), bytesLeft, 0);
			if (bytesRead < 1)
				continue;
			bytesLeft -= bytesRead;
		}

		switch (buffer[0])
		{
		case 0x01:
		{
			writeBytes(buffer, 2, 0);

			if (rs422InputBuffer.head + 1 == rs422InputBuffer.tail)
			{
				printf("Error: Buffer full\n");
				arguments->running = 0;
				continue;
			}

			rs422InputBuffer.buffer[rs422InputBuffer.head] = buffer[1];

			if (rs422InputBuffer.head + 1 == BUFFER_SIZE)
				rs422InputBuffer.head = 0;
			else
				rs422InputBuffer.head++;
		}
		break;

		case 0x80:
		{
			unsigned char outputBuffer[2] = {0x80, 0x00}; // Empty
			if (rs422OutputBuffer.head != rs422OutputBuffer.tail)
			{
				outputBuffer[1] = 0x40; // Not empty
			}
			writeBytes(outputBuffer, 2, 0);
		}
		break;

		case 0x81:
		{
			unsigned char outputBuffer[2] = {0x81, 0x00}; // Empty
			if (rs422OutputBuffer.head != rs422OutputBuffer.tail)
			{
				outputBuffer[1] = rs422OutputBuffer.buffer[rs422OutputBuffer.tail];

				if (rs422OutputBuffer.tail + 1 == BUFFER_SIZE)
					rs422OutputBuffer.tail = 0;
				else
					rs422OutputBuffer.tail++;
			}
			writeBytes(outputBuffer, 2, 0);
		}
		break;

		default:
        // error catch
			printf("Error: RS422 Thread %d is an unknown byte\n", buffer[0]);
			arguments->running = 0;
			break;
		}
	}

	printf("RS422 Thread Stopped.\n");
}