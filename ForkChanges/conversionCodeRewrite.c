
// This code hopes to simplify and isolate the RS422 conversion code

//TODO: opening and closing thread
//TODO: Properly applying the read/write functions

//public structs and read/writing funcs
//to be called in opened thread

// Defines struct CircularBuffer that is then instanced as input/output buffer
#define BUFFER_SIZE 1024
#define TIMEOUT_SELECT 1000

typedef struct
{
    //buffer used to prevent overflow errors
    //array called buffer, array is set to buffer_size
    unsigned char buffer[BUFFER_SIZE];
    //int vars used to read/write data in the buffer
    int head;
    int tail;
} CircularBuffer;

//struct instances for both input and output
CircularBuffer rs422InputBuffer;
CircularBuffer rs422OutputBuffer;

// function to read incoming bytes
// to be called in thread
//info: int function type, parses pointer to buffer and one var
int readBytes(unsigned char *buffer, int amount)
{
    //if struct vars (defined above) equal each other, timeout for 1000 microseconds and return nothing
    //in english: just pauses the byte reading if the circular buffer is empty/no data
    if (rs422InputBuffer.head == rs422InputBuffer.tail)
    {
        usleep(TIMEOUT_SELECT * 1000);
        return 0;
    }

    // defines int SIZE to be buffer_size & the inputbuffer.head minus .tail
    // size is how much data inside buffer
    int size = BUFFER_SIZE + rs422InputBuffer.head - rs422InputBuffer.tail;

    // if inputbuffer.head is more than or equal to inputbuffer.tail
    // minus inputbuffer.tail from inputbuffer.head & assign this value to var size
    // if head is in front of tail in buffer, buffer size is actually head - minus tail
    if (rs422InputBuffer.head >= rs422InputBuffer.tail){
        size = rs422InputBuffer.head - rs422InputBuffer.tail;
    }

    //then copy the memory values of rs422inputbuffer buffer (string inside predefined struct) into buffer ?
    //using size to indicate how many bytes to copy ?

    // Copying memory values into buffer. Looks at start first, then offsets by tail value, then copies size value. Copies whole buffer.
    // in english: copying data from circular buffer into normal buffer
    memcpy(buffer, &rs422InputBuffer.buffer[rs422InputBuffer.tail], size);

    // if buffer is full, ie cant be used without looping, moves tail along the buffer accounting for the loop
    if (rs422InputBuffer.tail + size > BUFFER_SIZE)
        rs422InputBuffer.tail = rs422InputBuffer.tail + size - BUFFER_SIZE;
    //moves tail along buffer if loop isnt needed
    else
        rs422InputBuffer.tail += size;
    return size;
}


// function to write outgoing bytes w/ pointer to buffer
// to be called in thread
int writeBytes(unsigned char *buffer, int amount)
{
    // if no data, return 0
    if (amount < 1)
        return 0;

    // if head + 1 is in same place in buffer as tail, ie no data in buffer, return 0
    if (rs422OutputBuffer.head + 1 == rs422OutputBuffer.tail)
        return 0;

    // if buffer is about to be full, do this
    // if data you want to write would overflow buffer, do this
    if (rs422OutputBuffer.head + amount > BUFFER_SIZE)
    {
        // buffer size - outputbuffer is how much is left
        int firstPart = BUFFER_SIZE - rs422OutputBuffer.head;
        // how much data left to written from the start of outputbuffer (circular buffer)
        int secondPart = amount - firstPart;
        // copying first part into circular buffer (whats left of in circular buffer)
        // copying data from pointer of .buffer inside outputbuffer struct, at the position of outputbuffer head
        // data its copying is from buffer ("working memory")
        memcpy(&rs422OutputBuffer.buffer[rs422OutputBuffer.head], buffer, firstPart);
        // copying second part (remaining data) into start of circular buffer
        memcpy(&rs422OutputBuffer.buffer[0], buffer + firstPart, secondPart);
        //moves head to end of second part (after writing)
        rs422OutputBuffer.head = secondPart;
    }
    else
    //else memcopy 
    {
        //copies buffer ("working memory") into output buffer (data ready to write to file) starting at head, amount is how far its writing
        //writes new data into output buffer starting at head
        memcpy(&rs422OutputBuffer.buffer[rs422OutputBuffer.head], buffer, amount);
        //move buffer head to where data (amount) ends
        rs422OutputBuffer.head += amount;
    }
    //return written data
    return amount;
}

//temp placeholder. This would be inside the YAC code that would start thread when data conversion is required
bool isRunning = true;

if isRunning = true; 
{
//start thread for conversion
//read/write bytes as applicable for values as read by readBytes
} else
return;


