# Communication Library
A small library that is used for communication between two applications ( a Producer and a Consumer ).
The producer creates messages, and use this library to send these messages to the consumer, which will read them.

### Demo
- This library uses a circular buffer to store and read all the messages. 
- These buffers are created with shared memory.
