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
    // defines thread w/ name i think
    pthread_t rs422ThreadID;
    // fairly simple int to make sure the thread keeps running  (if running = 1 (do while))
	int running = 1;

 	if (rs422Mode)
	{
        // defines an instance of a RS422ThreadArguments struct called arguments
		RS422ThreadArguments arguments = {0};
        // defines values for things inside struct (forgot term)
		arguments.fd = serialIO;
		arguments.running = &running;
        // creates the thread and parses data into it
		pthread_create(&rs422ThreadID, NULL, rs422Thread, &arguments);
	}

// defines data type of variables inside struct
// in proper code this goes above the defining an instance of said struct (i think)
// but since this is just a commented reference/draft thing this is fine
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

 // Opens a void w/ pointer called rs422Thread (this was defined earlier). 
 // This in itself contains a void pointer called vargp. 
 // Chatgpt tells me this is apparently a typical way to make a thread
 // void *vargp is a pointer to the threads arguments, this returns a void pointer
 // this can be used to return data, presumeably the converted card data
void *rs422Thread(void *vargp)
{
    //idk
    //defines var (?) RS422ThreadArguments with a pointer called arguments
    //arguments is a void (?) pointer containing vargp (whatever that is)

	//new explanation with chatgpt helping (so might be wrong)
	//arguments (struct instance defined earlier) is cast from vargp
	//(vargp is not in this isolated code) into a pointer called RS422ThreadArguments
	//not sure what RS422ThreadArguments is doing?
	RS422ThreadArguments *arguments = (RS422ThreadArguments *)vargp;

	//chatgpt says:
	//initilizes variables 
	//two buffers have the head/tail data pointers set to 0
	//these are used to store input/output data

	//what i think:
	//head reads the first byte of input data
	//tail reads the last byte of input data
	//these functions are intilized here and set to 0
	rs422InputBuffer.head = rs422OutputBuffer.head = 0;
	rs422InputBuffer.tail = rs422OutputBuffer.tail = 0;

	//self explanatory, runs the loop as long as the 'running' var in arguments struct is NOT zero
	//basically runs loop until said otherwise
	while (arguments->running)
	{
		//assigns a buffer string that is 3 bytes long
		unsigned char buffer[2];
		//int vars for conversion logic
		int bytesLeft = 2;

		int bytesRead = 0;

		//reads in a loop, trying to read 2 bytes from RS422 interface
		//reads in a loop until it has read 2 bytes, using bytesLeft for this
		//if it reads less than 1 byte, skip to next loop iteration
		while (bytesLeft > 0)
		{
			bytesRead = readBytes(buffer + (2 - bytesLeft), bytesLeft, 0);
			if (bytesRead < 1)
				continue;
			bytesLeft -= bytesRead;
		}


		// I HAVE NO CLUE WHAT IS HAPPENING FOR THIS ENTIRE THING BELOW

		//decides what to do with the data after reading, according to the values within the buffer
		//uses buffer value 0
		switch (buffer[0])
		{
		case 0x01: // 0x01 = 00000001
		{
			//writes the two read bytes back to the interface (?)
			writeBytes(buffer, 2, 0);

			//error catch if head buffer is ahead of the tail buffer (?)
			if (rs422InputBuffer.head + 1 == rs422InputBuffer.tail)
			{
				printf("Error: Buffer full\n");
				arguments->running = 0;
				continue;
			}
			//otherwise stores the second byte (buffer[1]) in the input buffer and updates the head pointer
			rs422InputBuffer.buffer[rs422InputBuffer.head] = buffer[1];
			//head pointer wraps around to 0 if adding to it would reach the buffer size
			//else +1
			if (rs422InputBuffer.head + 1 == BUFFER_SIZE)
				rs422InputBuffer.head = 0;
			else
				rs422InputBuffer.head++;
		}
		break;
		// ill be honest I dont have a clue
		case 0x80: // 0x80 = 10000000
		{
			unsigned char outputBuffer[2] = {0x80, 0x00}; // Empty
			if (rs422OutputBuffer.head != rs422OutputBuffer.tail)
			{
				outputBuffer[1] = 0x40; // Not empty
			}
			writeBytes(outputBuffer, 2, 0);
		}
		break;
		//sends data from the output buffer (ie sends the result of the conversion to where you want)
		//i think
		case 0x81: // 0x81 = 10000001
		{
			unsigned char outputBuffer[2] = {0x81, 0x00}; // Empty
			if (rs422OutputBuffer.head != rs422OutputBuffer.tail)
			{
				outputBuffer[1] = rs422OutputBuffer.buffer[rs422OutputBuffer.tail];

				//same as above with the outputbuffer.head
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
		//if the first byte is an unknown value (NOT 0x01, 0x80 or 0x81, prints error and stops thread)
			printf("Error: RS422 Thread %d is an unknown byte\n", buffer[0]);
			arguments->running = 0;
			break;
		}
	}
	// thread stopped, prints a message indicating such
	printf("RS422 Thread Stopped.\n");
}