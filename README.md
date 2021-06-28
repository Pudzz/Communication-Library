# Communication Library
A small library that is used for communication between two applications ( a Producer and a Consumer ).
The producer creates messages, and use this library to send these messages to the consumer, which will read them.

### Demo
- This library uses a circular buffer to store and read all the messages. 
- These buffers are created with shared memory.

To test the demo, open up the batchfile, rename the path to your own local path and set your own variables for each buffer.

#### Example in batchfile
[ START Shared.exe producer 1 64 20000 random ]

- 1 = The delay after each created or read message
- 64 = Size of the shared memory 
- 20000 = Number of messages to produce and read
- random = Gives a random size in bytes for each created message [Can put in a number if desired]
