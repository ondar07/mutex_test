// mutex_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>

#define THREADS_COUNT 100
#define ITERATIONS_COUNT 10

int global = 0;
HANDLE ghMutex;	// lock for @global

DWORD WINAPI thread_routine(LPVOID);

int main(void)
{
	HANDLE aThread[THREADS_COUNT];
	DWORD ThreadID;
	int i;

	// Create a mutex with no initial owner
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (ghMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	// Create worker threads
	for (i = 0; i < THREADS_COUNT; i++)
	{
		aThread[i] = CreateThread(
			NULL,       // default security attributes
			0,          // default stack size
			(LPTHREAD_START_ROUTINE)thread_routine,
			NULL,       // no thread function arguments
			0,          // default creation flags
			&ThreadID); // receive thread identifier

		if (aThread[i] == NULL)
		{
			printf("CreateThread error: %d\n", GetLastError());
			return 1;
		}
	}

	// Wait for all threads to terminate

	WaitForMultipleObjects(THREADS_COUNT, aThread, TRUE, INFINITE);

	// Close thread and mutex handles

	for (i = 0; i < THREADS_COUNT; i++)
		CloseHandle(aThread[i]);

	CloseHandle(ghMutex);

	printf("global=%d (should equal be 1000)\n", global);

	return 0;
}

DWORD WINAPI thread_routine(LPVOID lpParam)
{
	// lpParam not used in this example
	UNREFERENCED_PARAMETER(lpParam);
	DWORD dwCount = 0, dwWaitResult;

	for (dwCount = 0; dwCount < ITERATIONS_COUNT; dwCount++)
	{
		// acquire lock
		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			INFINITE);  // no time-out interval

		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:

			global++;

			// release lock
			if (!ReleaseMutex(ghMutex))
			{
				printf("ERROR can't release mutex\n");
			}
			break;

			// The thread got ownership of an abandoned mutex
			// The database is in an indeterminate state
		case WAIT_ABANDONED:
			printf("WAIT_ABANDONED\n");
			return FALSE;
		}
	}
	return TRUE;
}
