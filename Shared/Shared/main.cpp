#include "Comlib.h"

void gen_random(char* s, const int len);

int main(int argc, char* argv[])
{
	Comlib* comlib = nullptr;
	std::string smName = "Filemap";
	std::string smMutexName = "MutexMap";

	size_t messageSize = 0;

	char* msg = nullptr;

	/* msg producer header to fill */
	MsgHeader* messageHeader = new MsgHeader;

	/* ctrlheader for controlbuffer */
	ControlHeader* ctrlHeader = new ControlHeader;

	size_t offset = 0;

	bool doneClosing = false;
	bool done = false;

	srand((unsigned)time(0));
	if (argc > 0)
	{
		/* Delay */
		DWORD delay = atoi(argv[2]);

		/* MemorySize size */
		DWORD memorySize = DWORD(atoi(argv[3]) *(1 << 10));

		/* Number of messages */
		size_t messages = atoi(argv[4]);

		msg = new char[memorySize];

		if (strcmp(argv[1], "producer") == 0) 
		{
			comlib = new Comlib(smName, smMutexName, memorySize, PRODUCER);

			for (size_t i = 0; i < messages; i++)
			{
				if (strcmp(argv[5], "random") == 0) 
				{
					messageSize = (size_t)(rand() % memorySize / 4) + 1;
					gen_random(msg, messageSize);
				}
				else 
				{
					messageSize = (size_t)atoi(argv[5]);
					gen_random(msg, messageSize);
				}

				messageHeader->msgSize = messageSize;

				while (!done)
				{
					Sleep(delay);
					done = comlib->SendMsg(messageHeader, ctrlHeader, msg, offset);

					if(done)
						std::cout << i + 1 << ". " << (char*)msg << std::endl;
				}
				
				done = false;
			}
		}

		
		if (strcmp(argv[1], "consumer") == 0) 
		{
			comlib = new Comlib(smName, smMutexName, memorySize, CONSUMER);

			for (size_t i = 0; i < messages; i++)
			{				
				while (!done)
				{
					Sleep(delay);
					done = comlib->RecieveMsg(messageHeader, ctrlHeader, msg, offset);

					if(done)
						std::cout << i + 1 << ". " << (char*)msg << std::endl;
				}

				done = false;
				
			}
		}
	}
	
	delete[] msg;
	delete messageHeader;
	delete ctrlHeader;
	delete comlib;
	
	return 0;	
}

void gen_random(char* s, const int len) {

	static const char alphanum[] = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

	for (auto i = 0; i < len; ++i)
	{
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len - 1] = 0;
}