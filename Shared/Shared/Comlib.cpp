#include "Comlib.h"

Comlib::Comlib(const std::string& sharedName, const std::string& mutexName, const size_t& bufferSize, TYPE type)
{
	hFilemap = 0;
	hCtrlmap = 0;

	smName = sharedName;
	smCtrlName = "smCtrl";
	smMutexName = mutexName;
	smBufferSize = bufferSize;
	ctrlBufferSize = sizeof(ControlHeader);

	process = type;

	/*
		Check process type and create a file mapping object
		and fileviews for msgbuffer and ctrlbuffer
	*/

	if (process == TYPE::PRODUCER) {

		/* Shared memory for msgbuffer */
		hFilemap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, smBufferSize, (LPCWSTR)smName.c_str());
		if (hFilemap == NULL)
			std::cout << "Failed to create file mapping object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "File mapping object already exists - It's shared\n";

		/* Fileview of msgbuffer */
		msgBuffer = MapViewOfFile(hFilemap, FILE_MAP_ALL_ACCESS, 0, 0, smBufferSize);
		if (msgBuffer == NULL)
			std::cout << "View of file mapping object for msgbuffer failed\n";


		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /


		/* Shared memory for controlbuffer */
		hCtrlmap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, ctrlBufferSize, (LPCWSTR)smCtrlName.c_str());
		if (hCtrlmap == NULL)
			std::cout << "Failed to create file mapping object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "File mapping object already exists - It's shared\n";

		/* Fileview of ctrlbuffer */
		ctrlBuffer = MapViewOfFile(hCtrlmap, FILE_MAP_ALL_ACCESS, 0, 0, ctrlBufferSize);
		if (ctrlBuffer == NULL)
			std::cout << "View of file mapping object for controlbuffer failed\n";


		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


		hMutex = CreateMutex(NULL, FALSE, (LPCWSTR)smMutexName.c_str());
		if (hMutex == NULL)
			std::cout << "Failed to creatint mutex object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "Mutex already exists - It's shared\n";


		std::cout << ": : : : : PRODUCER CREATED : : : : :\n\n";
		//Sleep(10);
	}

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	if (process == TYPE::CONSUMER) {

		/* Shared memory for msgbuffer */
		hFilemap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, smBufferSize, (LPCWSTR)smName.c_str());
		if (hFilemap == NULL)
			std::cout << "Failed to create file mapping object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "File mapping object already exists - It's shared\n";

		/* Fileview of msgbuffer */
		msgBuffer = MapViewOfFile(hFilemap, FILE_MAP_ALL_ACCESS, 0, 0, smBufferSize);
		if (msgBuffer == NULL)
			std::cout << "View of file mapping object for msgbuffer failed\n";


		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * /


		/* Shared memory for controlbuffer */
		hCtrlmap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, ctrlBufferSize, (LPCWSTR)smCtrlName.c_str());
		if (hCtrlmap == NULL)
			std::cout << "Failed to create file mapping object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "File mapping object already exists - It's shared\n";

		/* Fileview of ctrlbuffer */
		ctrlBuffer = MapViewOfFile(hCtrlmap, FILE_MAP_ALL_ACCESS, 0, 0, ctrlBufferSize);
		if (ctrlBuffer == NULL)
			std::cout << "View of file mapping object for controlbuffer failed\n";


		/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


		hMutex = CreateMutex(NULL, FALSE, (LPCWSTR)smMutexName.c_str());
		if (hMutex == NULL)
			std::cout << "Failed to creatint mutex object\n";
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			std::cout << "Mutex already exists - It's shared\n";

		std::cout << ": : : : : CONSUMER CREATED : : : : :\n\n";
	}
}

Comlib::~Comlib()
{
	/* Closing msgbuffer stuff */
	UnmapViewOfFile(msgBuffer);
	
	if (hFilemap) 
	{
		CloseHandle(hFilemap);		
	}
	
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	/* Closing ctrlbuffer stuff */
	UnmapViewOfFile(ctrlBuffer);
	
	if (hCtrlmap) 
	{
		CloseHandle(hCtrlmap);		
	}
	
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	/* Closing mutex stuff */
	if (hMutex) 
	{
		CloseHandle(hMutex);		
	}
	
}

bool Comlib::SendMsg(MsgHeader* header, ControlHeader* ctrl, const void* msg, size_t& offset)
{
	bool result = false;

	if (!restarting)
		WaitForSingleObject(hMutex, INFINITE);

	if (restarting)
	{		
		memcpy(ctrl, (char*)ctrlBuffer, sizeof(ControlHeader));

		if (ctrl->processID == 2 && ctrl->offsetCon == 0)
		{
			WaitForSingleObject(hMutex, INFINITE);

			offset = 0;
			ctrl->offsetPro = offset;
			ctrl->producerSent = 0;
			ctrl->firstMessage = 0;
			memcpy((char*)ctrlBuffer, ctrl, sizeof(ControlHeader));
						
			restarting = false;
		}
	}

	memcpy(ctrl, (char*)ctrlBuffer, sizeof(ControlHeader));

	if (ctrl->processID != 1)
	{
		if (sizeof(MsgHeader) + header->msgSize + offset < (smBufferSize - (size_t)1))
		{
			if (sizeof(MsgHeader) + header->msgSize + offset >= ctrl->offsetCon)
			{
				memcpy((char*)msgBuffer + offset, header, sizeof(MsgHeader));
				offset += sizeof(MsgHeader);

				memcpy((char*)msgBuffer + offset, msg, header->msgSize);
				offset += header->msgSize;

				if (ctrl->producerSent == 0)
					ctrl->firstMessage = 1;

				ctrl->producerSent += 1;

				ctrl->processID = 0;

				ctrl->offsetPro = offset;
				memcpy((char*)ctrlBuffer, ctrl, sizeof(ControlHeader));

				result = true;
			}
			else
			{
				result = false;
			}
		}
		else
		{
			ctrl->processID = 1;
			memcpy((char*)ctrlBuffer, ctrl, sizeof(ControlHeader));

			result = false;
			restarting = true;
		}
	}
	else
	{
		result = false;
	}

	ReleaseMutex(hMutex);

	if (restarting)
		Sleep(10);

	return result;
}

bool Comlib::RecieveMsg(MsgHeader* readMsg, ControlHeader* ctrl, char* msg, size_t& offset)
{
	bool result = false;

	if (!restarting)
		WaitForSingleObject(hMutex, INFINITE);

	if (restarting)
	{		
		memcpy(ctrl, (char*)ctrlBuffer, sizeof(ControlHeader));

		if (ctrl->processID == 0 || ctrl->processID == 1)
		{
			WaitForSingleObject(hMutex, INFINITE);			
			restarting = false;
		}
	}	

	memcpy(ctrl, (char*)ctrlBuffer, sizeof(ControlHeader));
	memcpy(readMsg, (char*)msgBuffer + offset, sizeof(MsgHeader));
	
	
	if (ctrl->processID != 2 && ctrl->firstMessage == 1)
	{
		if (ctrl->consumerRead < ctrl->producerSent)
		{
			if (sizeof(MsgHeader) + readMsg->msgSize + offset < (smBufferSize - (size_t)1))
			{
				if (sizeof(MsgHeader) + readMsg->msgSize + offset <= ctrl->offsetPro)
				{
					
					memcpy(readMsg, (char*)msgBuffer + offset, sizeof(MsgHeader));
					offset += sizeof(MsgHeader);

					memcpy(msg, (char*)msgBuffer + offset, readMsg->msgSize);
					offset += readMsg->msgSize;

					ctrl->consumerRead += 1;

					ctrl->offsetCon = offset;
					memcpy((char*)ctrlBuffer, ctrl, sizeof(ControlHeader));

					result = true;
				}
			}
		}
		else
		{		
			if (ctrl->processID == 1)
			{
				offset = 0;
				ctrl->offsetCon = offset;
				ctrl->processID = 2;
				ctrl->consumerRead = 0;
				memcpy((char*)ctrlBuffer, ctrl, sizeof(ControlHeader));

				result = false;
				restarting = true;
			}
			else
			{
				result = false;
			}
		}
	}
	else
	{
		result = false;
	}
		

	ReleaseMutex(hMutex);

	if (restarting)
		Sleep(10);

	return result;
}