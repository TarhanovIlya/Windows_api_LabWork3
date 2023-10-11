#include <iostream>
#include<Windows.h>

//Marker start index from 1, not from 0


//static param
int ArraySize = 0;
int* Array;

//events
HANDLE StartAll;
HANDLE DecicionMade;
HANDLE MarkerEndedWork;
HANDLE* AllCreated;
//


HANDLE* hTArr;
DWORD* TIDArr;

HANDLE* Ev_MARKERS_STOPPED_Array;
HANDLE* Ev_MARKERS_STARTED_ARRAY;


bool* MarkedStoppedBoolArray;






CRITICAL_SECTION cs_Console_cout;





void FillWithZero(int* array, int arraySize) {

	for (int i = 0; i < arraySize; i++) {
		array[i] = 0;
	}


}

DWORD WINAPI marker(LPVOID param) {


	int markerNomer = static_cast<int>(reinterpret_cast<uintptr_t>(param));

	

	
	

	

	

	srand(markerNomer);
	int MarkedElemCount = 0;
	int* MarkedElem = new int[ArraySize];
	FillWithZero(MarkedElem, ArraySize);

	while (1) {

		SetEvent(AllCreated[markerNomer - 1]);
		SetEvent(Ev_MARKERS_STARTED_ARRAY[markerNomer - 1]);
		WaitForSingleObject(StartAll, INFINITE);

		
		int num = rand() % ArraySize;
		if (Array[num] == 0) {
			Sleep(5);
			Array[num] = markerNomer;
			Sleep(5);
			MarkedElemCount++;
			MarkedElem[num] = 1;
		}
		else {
			

			SetEvent(Ev_MARKERS_STOPPED_Array[markerNomer - 1]);


			
			EnterCriticalSection(&cs_Console_cout);
			std::cout << '\n';
			std::cout << "marker n=" << markerNomer <<"\n";
			std::cout << "amount of marked elem=" << MarkedElemCount << "\n";
			std::cout << "index of stop-elem= " << num << "\n";
			std::cout << '\n';
			//std::cout << "// DEBUG  (Marker) Array from marker: "; //debug
			//for (int i = 0; i < ArraySize; i++) {
			//	std::cout << Array[i] << " ";
			//}
			//std::cout << "\n";
			LeaveCriticalSection(&cs_Console_cout);

			
			
			WaitForSingleObject(DecicionMade, INFINITE);
			//std::cout << "// DEBUG  (Marker)  Wait DecicionMade done \n "<<"\n"<<"\n"; //debug
			
			ResetEvent(Ev_MARKERS_STARTED_ARRAY[markerNomer - 1]);
			ResetEvent(Ev_MARKERS_STOPPED_Array[markerNomer - 1]);
			
			
			
			//std::cout << "// DEBUG  (Marker)  DecicionMade was reset \n"<<"\n"<<"\n"; //debug
			if (MarkedStoppedBoolArray[markerNomer -1] == true) {
				
				for (int i = 0; i < ArraySize; i++) {
					if (MarkedElem[i] != 0) {
						Array[i] = 0;
					}
				}

				SetEvent(Ev_MARKERS_STOPPED_Array[markerNomer - 1]);
				SetEvent(Ev_MARKERS_STARTED_ARRAY[markerNomer - 1]);
				SetEvent(MarkerEndedWork);

				//std::cout << "// DEBUG   MarkerEndedWork was set \n"; //debug
				
				
				
				return 0;
			}
			else
			{
				
			}

		}

	}

}



int main() {
	
	InitializeCriticalSection(&cs_Console_cout);

	
	
	//Data input
	
		std::cout << "Enter array size: ";
		std::cin >> ArraySize;
		//ArraySize = 5;		//debug
		//std::cout << "\n";

		Array = new int[ArraySize];
		FillWithZero(Array, ArraySize);

		int MarkerNum = 0;
		std::cout << "Enter number of markers: ";
		std::cin >> MarkerNum;
		//MarkerNum = 2;		//debug
		//std::cout << "\n";

	
	
	
	//init MarkerStoppedBoolArray with False
		
		MarkedStoppedBoolArray = new bool[MarkerNum];
		for (int i = 0; i < MarkerNum; i++) {
			MarkedStoppedBoolArray[i] = FALSE;
		}
	


	// create thread handle array; thread id array
		
		hTArr = new HANDLE[MarkerNum];
		TIDArr = new DWORD[MarkerNum];
	


	//Create events
		
		DecicionMade = CreateEventW(NULL, TRUE, FALSE, L"decicion");
		Ev_MARKERS_STOPPED_Array = new HANDLE[MarkerNum];
		Ev_MARKERS_STARTED_ARRAY = new HANDLE[MarkerNum];
		AllCreated = new HANDLE[MarkerNum];
		StartAll = CreateEvent(NULL, TRUE, FALSE, L"StartAll");
		if (StartAll == NULL) {
			std::cout << "DEBUG    ERROE: StartAll == 0";					//debug
			return 0;
		}
		MarkerEndedWork = CreateEventW(NULL, FALSE, FALSE, L"EndedWork");

		for (int i = 0; i < MarkerNum; i++) {
			//	std::cout << i << " ";//debug

			Ev_MARKERS_STOPPED_Array[i] = CreateEvent(NULL, TRUE, FALSE, L"markedStopArray" + (wchar_t)i);
			if (Ev_MARKERS_STOPPED_Array[i] == NULL) {
				std::cout << "DEBUG    ERROE: Ev_MARKERS_STOPPED_Array["<<i<<"] == 0";					//debug
				return 0;
			}

		}


		for (int i = 0; i < MarkerNum; i++) {
			//	std::cout << i << " ";//debug

			Ev_MARKERS_STARTED_ARRAY[i] = CreateEvent(NULL, TRUE, FALSE, L"markedStartArray" + (wchar_t)i);
			if (Ev_MARKERS_STARTED_ARRAY[i] == NULL) {
				std::cout << "DEBUG    ERROE: Ev_MARKERS_STARTED_ARRAY[" << i << "] == 0";					//debug
				return 0;
			}

		}


		for (int i = 0; i < MarkerNum; i++) {
			//	std::cout << i << " ";//debug

			AllCreated[i] = CreateEvent(NULL, TRUE, FALSE, L"created"+(wchar_t)i);
			DWORD errorCode = GetLastError(); 													//debug
			if (AllCreated[i] == NULL) {
				std::cout << "DEBUG    ERROE: AllCreated[" << i << "] == 0";					//debug
				return 0;
			}

		}
		//if ((sizeof(AllCreated) / sizeof(HANDLE) != MarkerNum)||(sizeof(AllCreated) / sizeof(HANDLE) == 0)) {
		//	std::cout << "DEBUG    ERROE: AllCreated size ="<< (sizeof(AllCreated) / sizeof(HANDLE))<<"\n"<<"\n";					//debug
		//	HANDLE ifOutOfBound0 = AllCreated[0];
		//	HANDLE ifOutOfBound1 = AllCreated[1];

		//	return 0;
		//}
	
		
		//create threads;
	for (int i = 0; i < MarkerNum; i++) {

		hTArr[i] = CreateThread(NULL, NULL, marker, (LPVOID)(i+1), NULL, &TIDArr[i]);
		if (hTArr[i] == NULL) {
			std::cout << "DEBUG    ERROE: hTArr[" << i << "] == 0";					//debug
			return 0;
		}

	}

	//check if all threads are created
	if (WaitForMultipleObjects(MarkerNum, AllCreated, TRUE, INFINITE) == WAIT_FAILED) {
		std::cout << "DEBUG    ERROR: WaitForMultipleObjects(MarkerNum, AllCreated, TRUE, INFINITE)  FAILED" << "\n" << "\n";					//debug
		DWORD errorCode = GetLastError();
		return 0;
	};

	////Make signal to start all markers;
	//if (!SetEvent(StartAll)) { std::cout << "// DEBUG   ERROR: PROBLEM OCCURED WHILE SETTING StartAll EVENT\n "<<"\n"<<"\n"; }					//debug


	//cycle
	bool StopProgram=false;
	int markerToStop = 0;


	while (1) {

		if (StopProgram == true) { break; }
		if (WaitForMultipleObjects(MarkerNum, Ev_MARKERS_STARTED_ARRAY, TRUE, INFINITE) == WAIT_FAILED) {
			std::cout << "DEBUG    ERROR:  WaitForMultipleObjects(MarkerNum, Ev_MARKERS_STARTED_ARRAY, TRUE, INFINITE)  FAILED" << "\n" << "\n";					//debug
			DWORD errorCode = GetLastError();
			return 0;
		}
		
		SetEvent(StartAll);
		DWORD errorCode = GetLastError();							//debug
		
		

		if (WaitForMultipleObjects(MarkerNum, Ev_MARKERS_STOPPED_Array, TRUE, INFINITE) == WAIT_FAILED) {
			std::cout << "DEBUG    ERROR:  WaitForMultipleObjects(MarkerNum, Ev_MARKERS_STOPPED_Array, TRUE, INFINITE)  FAILED";
			DWORD errorCode = GetLastError();
			return 0;
		}
		//std::cout << "// DEBUG   WaitFor EvMARKEDSTOPPED_Array made\n"<<"\n"<<"\n";//debug

				
			//display array 1
		EnterCriticalSection(&cs_Console_cout);

		std::cout << "(Main) displaying array(1)  : ";
		for (int i = 0; i < ArraySize; i++) {
			std::cout << Array[i] << ' ';
		}
		std::cout << "\n";
		
		std::cout << "input what marker will stop working:";
		std::cin >> markerToStop;
		LeaveCriticalSection(&cs_Console_cout);

		MarkedStoppedBoolArray[markerToStop-1] = true;

		if (!SetEvent(DecicionMade)) { std::cout << "// DEBUG   ERROR: SOMETHING HAPPENED WHILE SetEvent(DecicionMade)\n" << "\n" << "\n"; }					//debug
		

		//std::cout << "// DEBUG   SetEvent Decicion made\n"<<"\n"<<"\n";//debug


		WaitForSingleObject(MarkerEndedWork, INFINITE);
		CloseHandle(hTArr[markerToStop-1]);
		
		ResetEvent(DecicionMade);
		ResetEvent(MarkerEndedWork);

		//std::cout << "// DEBUG   WaitFor MarkerEndedWork made\n";//debug


		//display array 2
		EnterCriticalSection(&cs_Console_cout);
		std::cout << "(Main) displaying array(2)  : ";
		for (int i = 0; i < ArraySize; i++) {
			std::cout << Array[i] << ' ';
		}
		std::cout << "\n\n";
		LeaveCriticalSection(&cs_Console_cout);

		


		StopProgram = true;
		for (int i = 0; i < MarkerNum; i++) {
			if (MarkedStoppedBoolArray[i] != TRUE) {
				StopProgram = false;
			}
		}
	}

	


	//closing handles, deleting arrays
	
	for (int i = 0; i < MarkerNum; i++) {
		CloseHandle(AllCreated[i]);
		CloseHandle(Ev_MARKERS_STARTED_ARRAY[i]);
		CloseHandle(Ev_MARKERS_STOPPED_Array[i]);
	}
	
	
	{
		CloseHandle(StartAll);
		CloseHandle(DecicionMade);
	    // by this point all Ev_MARKERS_STOPPED_Array members point to AlwaysSignaled Handle;
		
		
		DeleteCriticalSection(&cs_Console_cout);

		delete Array;
		delete hTArr;
		delete MarkedStoppedBoolArray;
	}

	std::cout << "\n\n\n              PROGRAMM STOPPED\n\n\n\n";

}