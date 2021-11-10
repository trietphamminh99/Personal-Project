#include<iostream>
#include<Vector> 
#include<Windows.h>
#include <thread>
using namespace std;
int playFieldWidth = 12;
int playFieldHeight = 18;
wstring tetromino[7];
	int screenWidth = 80;
	int screenHeight = 30;
	unsigned char* pField = nullptr;
	
	int Rotate(int x, int y, int r)
	{
		switch (r % 4)
		{
		case 0: return y * 4 + x;
		case 1: return 12 + y - (x * 4);
		case 2: return 15 - (y * 4) - x;
		case 3: return 3 - y + (x * 4);
		}
		return 0;
	}
	bool DoesItFit(int Tetromino, int Rotation, int PosX, int PosY)
	{
	
		for (int px = 0; px < 4 ; px ++)
			for (int py = 0; py < 4; py++)
			{
				// get index into piece 
				int pi = Rotate(px, py, Rotation);
				// get index into field
				int fi = (PosY + py) * playFieldWidth + (PosX + px);
				if (PosX + px >= 0 && PosX + px < playFieldWidth)
				{
					if (PosY + py >= 0 && PosY + py < playFieldHeight)
					{
						if (tetromino[Tetromino][pi] == L'X' && pField[fi] != 0)
							return false;
					}
				}


			}
		return true;
	}
	void SetWindowSize(SHORT width, SHORT height) {
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		SMALL_RECT WindowSize;
		WindowSize.Top = 0;
		WindowSize.Left = 0;
		WindowSize.Right = width;
		WindowSize.Bottom = height;
		SetConsoleWindowInfo(hStdOut, 1, &WindowSize);
	}
int main(){
	// tetromino
	{
		tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
		tetromino[1].append(L"..X..XX...X.....");
		tetromino[2].append(L".....XX..XX.....");
		tetromino[3].append(L"..X..XX..X......");
		tetromino[4].append(L".X...XX...X.....");
		tetromino[5].append(L".X...X...XX.....");
		tetromino[6].append(L"..X...X..XX.....");
	}
	SetWindowSize(screenWidth-1, screenHeight );
	pField = new unsigned char[playFieldWidth * playFieldHeight]; // Create play field buffer
	for (int x = 0; x < playFieldWidth; x++) // Board Boundary
		for (int y = 0; y < playFieldHeight; y++)
			pField[y * playFieldWidth + x] = (x == 0 || x == playFieldWidth - 1 || y == playFieldHeight - 1) ? 9 : 0;
	
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	


	//Game Logic stuff
	int CurrentPiece = 0;
	int nextPiece = 0;
	int CurrentRotation = 0;
	int CurrentX = playFieldWidth / 2;
	int CurrentY = 0;
	int Speed = 20, speedCounter = 0;
	bool forceDown = false;
	bool rotateHold = false;
	bool Key[4];
	bool gameOver = false;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vline;
	while (!gameOver)
	{
		//Game Timeing 
		this_thread::sleep_for(50ms);//Game Tick
		speedCounter++;
		forceDown = (speedCounter == Speed);

		//InPut
		for (int k = 0; k < 4; k++)
		{
			Key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		//GameLogic
		///player movement
		{

			if (Key[1])
			{
				if (DoesItFit(CurrentPiece, CurrentRotation, CurrentX -1, CurrentY) ){
					CurrentX = CurrentX-1;
				}
			}
			if (Key[0])
			{
				if (DoesItFit(CurrentPiece, CurrentRotation, CurrentX +1 , CurrentY)) {
					CurrentX = CurrentX + 1;
				}
			}
			if (Key[2])
			{
				if (DoesItFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY+1)) {
					CurrentY = CurrentY + 1;
				}
			}
			if (Key[3])
			{
				if (!rotateHold && DoesItFit(CurrentPiece, CurrentRotation +1 , CurrentX, CurrentY)) {
					CurrentRotation += 1;
				}
					rotateHold = true;
			}
			else {
				rotateHold = false;
			}
		}
		if (forceDown)
		{
			//can it fit furrder down
			if (DoesItFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY + 1))
				CurrentY++; // It can, so do it
			// it cant
			else
			{
				//lock the current piece in the field 
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[CurrentPiece][Rotate(px, py, CurrentRotation)] == L'X')
							pField[(CurrentY + py) * playFieldWidth + (CurrentX + px)] = CurrentPiece + 1;
					}
				}
				nPieceCount++;
				if (nPieceCount %10 == 0)
					if (nPieceCount >= 10) Speed--;

				// check have we got any line
				{

					for (int py = 0; py < 4; py++)
					if (CurrentY + py < playFieldHeight -1 )
					{
						bool line = true;
						for (int px = 1; px < playFieldWidth - 1; px++)
							line &= (pField[(CurrentY + py) * playFieldWidth + px]) != 0;

						if (line)
						{
							//set to '=', remove line
							for (int px = 1; px < playFieldWidth - 1; px++)
							{
								pField[(CurrentY + py) * playFieldWidth + px] = 8;
							}
							vline.push_back(CurrentY + py);
						}
						nScore += 25;
						if (!vline.empty()) nScore += (1 << vline.size()) * 100;
					}
				}
				// choose next piece 
				
				{
				CurrentX = playFieldWidth / 2;
				CurrentY = 0;
				CurrentRotation = 0;
				CurrentPiece = rand() % 7;
				}
				// if piece dose not fit
				gameOver = !DoesItFit(CurrentPiece, CurrentRotation, CurrentX, CurrentY);
			}
			speedCounter = 0;
		}
		//Render out




		//Draw border 
	
		{
		
		for (int x = 0; x < playFieldWidth; x++)
			for (int y = 0; y < playFieldHeight; y++)
				screen[(y+2) * screenWidth + (x + 10)] = L" ABCDEFG=#"[pField[y * playFieldWidth + x]];

		}
		// Draw Curretn Piece
		{

			for (int px = 0; px < 4; px++)
			{
				for (int py = 0; py < 4; py++)
				{
					if (tetromino[CurrentPiece][Rotate(px, py, CurrentRotation)] == L'X')
						{
						screen[(CurrentY + py + 2) * screenWidth + (CurrentX + px + 10)] = CurrentPiece + 65;
						}

				}
			}
		}
		// Draw Score
		swprintf_s(&screen[2 * screenWidth + playFieldWidth + 20], 16, L"SCORE: %8d", nScore);
		if (!vline.empty())
		{
			//display Frame
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth* screenHeight, { 0,0 }, & dwBytesWritten);
			this_thread::sleep_for(400ms); //delay a bit
			for (auto& v : vline)
			{
				for (int px = 1; px < playFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * playFieldWidth + px] = pField[(py - 1) * playFieldWidth + px];
						pField[px] = 0;
					}
				}
			}
			vline.clear();
		}
			

		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}
	// GameOver
	CloseHandle(hConsole);
	cout << "Game Over!! Score: " << nScore << endl;
	system("pause");

}