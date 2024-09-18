
// This code hopes to simplify and isolate the RS422 conversion code

//TODO: opening and closing thread
//TODO: check void is properly defined, change if needed

//public structs and read/writing funcs
//to be called in opened thread

//void might need to be a return type?
public void cardConversion {

// Defines struct CircularBuffer that is then instanced as input/output buffer
typedef struct
{
    //buffer used to prevent overflow errors
    //string (array of chars) called buffer, array is set to a value called buffer_size?
    unsigned char buffer[BUFFER_SIZE];
    //int vars used to read/write data
    int head;
    int tail;
} CircularBuffer;

//struct instances
CircularBuffer rs422InputBuffer;
CircularBuffer rs422OutputBuffer;

// function to read incoming bytes
// to be called in thread
//info: int function type, parses one pointer and one var (buffer and amount respectively)
int readBytes(unsigned char *buffer, int amount)
{
    //if struct vars (defined above) equal each other, timeout for 1000 microseconds and return nothing
    //in english: just pauses the byte reading if the input buffer is all the same (only if its empty?)
    if (rs422InputBuffer.head == rs422InputBuffer.tail)
    {
        usleep(TIMEOUT_SELECT * 1000);
        return 0;
    }

    // defines int SIZE to be buffer_size & the inputbuffer.head minus .tail
    int size = BUFFER_SIZE + rs422InputBuffer.head - rs422InputBuffer.tail;

    // if inputbuffer.head is more than or equal to inputbuffer.tail
    // minus inputbuffer.tail from inputbuffer.head & assign this value to var size
    if (rs422InputBuffer.head >= rs422InputBuffer.tail)

        size = rs422InputBuffer.head - rs422InputBuffer.tail;

    //then copy the memory values of rs422inputbuffer buffer (string inside predefined struct) into buffer ?
    //using size to indicate how many bytes to copy ?
    memcpy(buffer, &rs422InputBuffer.buffer[rs422InputBuffer.tail], size);

    if (rs422InputBuffer.tail + size > BUFFER_SIZE)
        rs422InputBuffer.tail = rs422InputBuffer.tail + size - BUFFER_SIZE;
    else
        rs422InputBuffer.tail += size;

    return size;
}


// function to write outgoing bytes
// to be called in thread
int writeBytes(unsigned char *buffer, int amount)
{
    
    if (amount < 1)
        return 0;

    if (rs422OutputBuffer.head + 1 == rs422OutputBuffer.tail)
        return 0;

    if (rs422OutputBuffer.head + amount > BUFFER_SIZE)
    {
        int firstPart = BUFFER_SIZE - rs422OutputBuffer.head;
        int secondPart = amount - firstPart;
        memcpy(&rs422OutputBuffer.buffer[rs422OutputBuffer.head], buffer, firstPart);
        memcpy(&rs422OutputBuffer.buffer[0], buffer + firstPart, secondPart);
        rs422OutputBuffer.head = secondPart;
    }
    else
    {
        memcpy(&rs422OutputBuffer.buffer[rs422OutputBuffer.head], buffer, amount);
        rs422OutputBuffer.head += amount;
    }
    return amount;
}

//temp placeholder. This would be inside the YAC code that would start thread when data conversion is required
bool isRunning = true;

if isRunning = true; 
{
//start thread for conversion
} else
return;

}

