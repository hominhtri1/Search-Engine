#include "RTree.h"
#include "BTreeNewsPath.h"
#include "BTreeStopWord.h"
#include "Synonym.h"
#include "STree.h"


void setDisplay()
{
	system("color f0");

	CONSOLE_FONT_INFOEX cfi;

	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 17;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;

	wcscpy_s(cfi.FaceName, L"Console");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions

	MoveWindow(console, r.left, r.top, 1200, 700, TRUE); // width, height

	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
}

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color)
{
	WORD wColor;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		wColor = (csbi.wAttributes & 0xF0) + (color & 0x0F);
		SetConsoleTextAttribute(hStdOut, wColor);
	}
	return;
}

//---------------------------

void intro()
{
	string arr_string[1] = { " WELCOME TO GROUP 5 MINI SEARCHING ENGINE " };

	HANDLE screen = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD pos[18] = { { 0,6 },{ 0,7 },{ 0,9 },{ 0,11 },{ 0,13 },{ 0,15 },
	{ 0,17 },{ 0,19 },{ 0,21 },{ 0,23 },{ 0,25 },{ 0,27 },
	{ 0,29 },{ 0,31 },{ 0,33 },{ 0,35 },{ 0,37 },{ 0,39 } };

	for (int j = arr_string[0].length(); j >= 0; j--)
	{
		for (int i = 0; i <= j; i = i++)
		{
			if (j >= 19)
				setColor(9);
			else
				setColor(5);
			SetConsoleCursorPosition(screen, pos[0]);
			cout << "" << endl;
			pos[0].X++;

			SetConsoleCursorPosition(screen, pos[0]);
			cout << arr_string[0][j] << endl;
			Sleep(1);
		}
		pos[0] = { (SHORT)0, (SHORT)(1) };
	}
}

void welcome()
{
	while (true)
	{
		system("cls");

		setDisplay();

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		string ans;

		gotoxy(2, 1);
		setColor(5); cout << "WELCOME TO GROUP 5 ";
		setColor(9); cout << "MINI SEARCH ENGINE";

		setColor(1);
		gotoxy(1, 3); cout << "[1] Insert news.";
		gotoxy(1, 4); cout << "[2] Search.";
		gotoxy(1, 5); cout << "[3] Delete history.";
		gotoxy(1, 6); cout << "[4] Close Search Engine.";

		setColor(12);
		gotoxy(1, 8); cout << "Press the number in [ ] to choose option. ";
		setColor(1);
		getline(cin, ans);

		while (ans != "1" && ans != "2" && ans != "3" && ans != "4")
		{
			setColor(12);
			gotoxy(1, 10); cout << "WRONG INPUT. PLEASE DO IT AGAIN. PRESS [enter].";

			while (cin.get() != '\n');
			system("cls");

			gotoxy(2, 1);
			setColor(5); cout << "WELCOME TO GROUP 5 ";
			setColor(9); cout << "MINI SEARCH ENGINE";

			setColor(1);
			gotoxy(1, 3); cout << "[1] Insert news.";
			gotoxy(1, 4); cout << "[2] Search.";
			gotoxy(1, 5); cout << "[3] Delete history.";
			gotoxy(1, 6); cout << "[4] Close Search Engine.";

			setColor(12);
			gotoxy(1, 8); cout << "Press the number in [ ] to choose option. ";
			setColor(1);
			getline(cin, ans);
		}

		int key = stoi(ans, NULL);

		switch (key)
		{
		case 1:
		{			
			//prepareBTreeDataStr();

			prepareBTreeSynonym();
			//prepareBTreeStopWord();

			/*for (int i = 12; i < 16; ++i)
			{
				int start = -1;
				int end = -1;

				prepareBTreeDataStr(start, end, i);

				cout << start << " " << end << endl;

				cout << "num" << endl;

				prepareBTreeData(start, end);

				cout << "paragraph" << endl;

				prepareParagraphPos(start, end);

				cout << "word" << endl;

				prepareWordPos(start, end);
			}*/

			gotoxy(1, 10); cout << "DONE INSERTION.";
			gotoxy(1, 11); cout << "Press [enter] to continue.";

			while (cin.get() != '\n');
			break;
		}
		case 2:
		{
			//auto started = std::chrono::high_resolution_clock::now();
			
			RNode* root = NULL;
			perform(root);

			//query();

			//auto done = std::chrono::high_resolution_clock::now();

			//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();

			//cout << endl;
			//while (cin.get() != '\n');
			break;
		}
		case 3:
		{
			ofstream fout;
			fout.open(rootPath + "storage - RTree.txt", ofstream::trunc);
			fout.close();

			string path = rootPath + "fileIndex.txt";

			if (remove(path.c_str()) == 0)
			{
				SetConsoleTextAttribute(hConsole, 252);
				gotoxy(1, 9);
				cout << "ERASED SUCCESSFULLY. Press [enter] to continue. \n\n";

				while (cin.get() != '\n');
			}
			break;
		}
		case 4: return;
		}
	}
}

void perform(RNode* &root)
{
	system("cls");
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	fstream RTree;
	ifstream fin;
	ofstream fout;

	int fileIndex;

	string query = "", ans = "";
	string path = rootPath + "storage - RTree.txt";

	string* top5 = new string[20];

	fin.open(rootPath + "fileIndex.txt");

	if (!fin.is_open()) fileIndex = -1;
	else fin >> fileIndex;

	fin.close();

	RTree.open(path, fstream::binary | fstream::out | fstream::in);

	if (!RTree.is_open())
	{
		fout.open(path);

		fout.close();

		RTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	SetConsoleTextAttribute(hConsole, 252);
	gotoxy(10, 0); cout << "Find: ";

	for (; ; )
	{
		if (_kbhit())
		{
			char c = _getch();

			switch (c)
			{
			case 13:
			{
				query = query + "`";
				insertTree(root, query, fileIndex, RTree);

				fout.open(rootPath + "fileIndex.txt");
				fout << fileIndex;
				fout.close();

				RTree.close();

				query = query.substr(0, query.length() - 1);
				show(query);
				
				break;
			}
			case 8:
			{
				if (query.length() == 1)
				{
					query = "";
					print(root, query, top5, false, RTree);
				}
				else
				{
					query = query.substr(0, query.length() - 1);
					print(root, query, top5, true, RTree);
				}

				SetConsoleTextAttribute(hConsole, 252);
				gotoxy(0, 0); cout << emptyStr;
				gotoxy(10, 0); cout << "Find: ";
				SetConsoleTextAttribute(hConsole, 240);
				gotoxy(16, 0); cout << query;

				break;
			}
			default:
			{
				string chStr = string(1, c);

				query = query + chStr;

				SetConsoleTextAttribute(hConsole, 240);
				gotoxy(16, 0); cout << query;

				print(root, query, top5, true, RTree);
			}
			}
			if (c == 13) break;
		}
	}

	delete[]top5;
}

void show(string query)
{
	while (true)
	{
		system("cls");

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		ifstream fin;

		prepareParagraphPos(0, 99);

		int* topFile = new int[20];
		int* available = new int[20];

		string titles[20];

		int curPara = 0;

		int count = topNewsFile(query, topFile);

		cout << "TOP 20 searching results. \n(Press [space] to see in details) or (Press [esc] to return to the intro page)";

		int y = 2;

		for (int i = 0; i < count; ++i)
		{
			int startPos[20];
			for (int i = 0; i < 20; ++i) startPos[i] = -1;

			posNumPair** hili = new posNumPair*[20];
			int paraNum = highlightPositions(topFile[i], query, startPos, hili);
			
			if (paraNum == 0)
			{
				int j = i;
				--count;
				while (j < count)
				{
					topFile[j] = topFile[j + 1];
					++j;
				}
				--i;
			}
			else
			{
				SetConsoleTextAttribute(hConsole, 252);
				gotoxy(0, ++y);

				string path = getNewsPaths(topFile[i]);
				cout << path;

				fin.open(path);
				getline(fin, titles[i], '\n');
				fin.close();

				if (i == 0) SetConsoleTextAttribute(hConsole, 14);

				gotoxy(0, ++y);
				if (titles[i].length() > 100) titles[i] = titles[i].substr(0, 100);
				cout << titles[i];
				++y;
			}
		}

		y = 4;

		for (; ; )
		{
			if (_kbhit())
			{
				char c = _getch();

				switch (c)
				{
				case 72:
				{
					SetConsoleTextAttribute(hConsole, 252);
					gotoxy(0, y);
					cout << titles[y - 2 * (curPara + 2)];
					if (curPara == 0) { curPara = count - 1; y = 2 * (curPara + 2) + 2; }
					else { y = y - 3; --curPara; }

					SetConsoleTextAttribute(hConsole, 14);
					gotoxy(0, y);
					cout << titles[y - 2 * (curPara + 2)];

					break;
				}
				case 80:
				{
					SetConsoleTextAttribute(hConsole, 252);
					gotoxy(0, y);
					cout << titles[y - 2 * (curPara + 2)];
					if (curPara == count - 1) { y = 4; curPara = 0; }
					else { y = y + 3; ++curPara; }

					SetConsoleTextAttribute(hConsole, 14);
					gotoxy(0, y);
					cout << titles[y - 2 * (curPara + 2)];

					break;
				}
				case 13:
				{
					press(topFile, curPara, query);

					break;
				}
				case 27:
				{
					delete[] topFile;
					return;
				}
				}

				if (c == 13) break;
			}
		}
	}
}

void press(int* topFile, int i, string query)
{
	system("cls");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	setDisplay();

	int startPos[20];
	for (int i = 0; i < 20; ++i) startPos[i] = -1;

	posNumPair** hili = new posNumPair*[20];

	int paraNum = highlightPositions(topFile[i], query, startPos, hili);

	string path = getNewsPaths(topFile[i]);

	ifstream fin;
	fin.open(path);

	stringstream buffer;
	buffer << fin.rdbuf();
	
	fin.close();

	string para = buffer.str();

	//delete_non_ascii(para);

	int count = 0;
	int* print = new int[para.length()];

	for (int x = 0; x < para.length(); ++x)
	{
		print[x] = -1;

		int j = 0;
		if (x == startPos[count])
		{
			while (para[x] != '\n' && x < para.length())
			{
				while ((para[x] != '\n') && (x != hili[count][j].pos) && x < para.length())
				{
					print[x] = 0;
					++x;
				}
				if (x == hili[count][j].pos)
				{
					int wordNum = 0;
					while ((x >= hili[count][j].pos) && x < para.length())
					{
						if (isHighlightDelimeter(para, x)) ++wordNum;
						if (wordNum == hili[count][j].wordNum) break;

						print[x] = 1;
						++x;
					}
					++j;
				}
			}
			++count;
		}
	}
	count = 0;

	for (int i = 0; i < para.length(); ++i)
	{		
		if (i == startPos[count])
		{
			while (para[i] != '\n' && i < para.length())
			{
				if (print[i] == 0)
				{
					SetConsoleTextAttribute(hConsole, 240);
					while (print[i] == 0) { cout << para[i];  ++i; }
				}
				if (print[i] == 1)
				{
					SetConsoleTextAttribute(hConsole, 14);
					while (print[i] == 1) { cout << para[i];  ++i; }
				}
			}

			++count;
			cout << endl << endl;
		}
	}

	for (int i = 0; i < paraNum; ++i) delete[] hili[i];
	delete[] hili;

	delete[]print;
	
	SetConsoleTextAttribute(hConsole, 240);
	cout << endl << "Press [enter] to go BACK.";
	while (cin.get() == '\n') return;
}

//---------------------------

bool find(RNode* root, string k, string* &top5, int& count, fstream& RTree)
{
	root = new RNode;
	readRTree(root, 0, RTree);

	int curPos = 0;

	while (true)
	{
		int ind = k[curPos] - NULL;

		if (-1 == root->child[ind])
		{
			delete root;
			return false;
		}

		RNode* next = new RNode;
		readRTree(next, root->child[ind], RTree);

		string tempStr((next->path).str);

		int i = 0;

		while (i < tempStr.length() && curPos < k.length() && tempStr[i] == k[curPos])
		{
			++i;
			++curPos;
		}

		string suffix = tempStr.substr(i, tempStr.length() - i + 1);

		if (k.length() == curPos)
		{
			string curStr = k + suffix;

			traverse(curStr, count, top5, next, RTree);

			delete next;
			delete root;
			return true;
		}
		else if (tempStr.length() == i)
		{
			readRTree(root, root->child[ind], RTree);
			delete next;
		}
		else
			return false;
	}
}

void traverse(string result, int& count, string* &top5, RNode* cur, fstream& RTree)
{
	if (count == 5) return;

	if (cur->isLeaf)
	{
		top5[count] = result.substr(0, result.length() - 1);
		++count;
		return;
	}

	for (int i = 0; i < 256; ++i)
	{
		if (cur->child[i] != -1)
		{
			RNode* tmp = new RNode;
			readRTree(tmp, cur->child[i], RTree);

			string newStr((tmp->path).str);
			newStr = result + newStr;
			traverse(newStr, count, top5, tmp, RTree);

			delete tmp;
		}
	}
}

void print(RNode* root, string str, string* &top5, bool goFind, fstream& RTree)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, 252);
	for (int i = 0; i < 5; ++i)
	{
		gotoxy(0, 3 + i); cout << "\r" << emptyStr;
	}

	int count = 0;
	bool exist = false;
	if (goFind) exist = find(root, str, top5, count, RTree);

	if (!exist) return;
	else
	{
		SetConsoleTextAttribute(hConsole, 252);
		gotoxy(0, 2); cout << "Recommendation: ";
		
		SetConsoleTextAttribute(hConsole, 240);
		for (int i = 0; i < count; ++i)
		{ gotoxy(16, i + 2); cout << top5[i] << "                                                                 "; }
	}
}

void writeRTree(RNode* cur, int curInd, fstream& RTree)
{
	RTree.seekp(curInd*(sizeof RNode), RTree.beg);

	RTree.write((char*)cur, sizeof RNode);
}

void readRTree(RNode* &cur, int fileName, fstream& RTree)
{
	RTree.seekp(fileName*(sizeof RNode), ios_base::beg);

	RTree.read((char*)cur, sizeof RNode);

	RTree.clear();
}

void insertTree(RNode* &cur, string k, int& fileIndex, fstream& RTree)
{
	cur = new RNode;
	readRTree(cur, 0, RTree);

	int curPos = 0;

	while (true)
	{
		if (curPos == k.length())
		{
			delete cur; return;
		}

		int ind = k[curPos] - NULL;

		if (-1 == cur->child[ind])
		{
			if (cur->fileName == -1) cur->fileName = ++fileIndex;
			cur->isLeaf = false;

			RNode* next = new RNode;
			strcpy_s((next->path).str, (k.substr(curPos, k.length() - curPos)).c_str());
			next->fileName = ++fileIndex;
			cur->child[ind] = next->fileName;

			writeRTree(cur, cur->fileName, RTree);
			writeRTree(next, next->fileName, RTree);

			delete next;
			delete cur;

			break;
		}

		RNode* next = new RNode;
		readRTree(next, cur->child[ind], RTree);

		string tempStr((next->path).str);
		int i = 0;

		delete next;

		while (i < tempStr.length() && tempStr[i] == k[curPos] && curPos < k.length())
		{
			++i;
			++curPos;
		}
		
		if (tempStr.length() != i)
		{
			int tempInd = tempStr[i] - NULL;
			
			RNode* newNode = new RNode;
			strcpy_s((newNode->path).str, (tempStr.substr(0, i).c_str()));
			newNode->fileName = ++fileIndex;
			newNode->isLeaf = false;

			RNode* nextNode = new RNode;
			readRTree(nextNode, cur->child[ind], RTree);
			strcpy_s((nextNode->path).str, (tempStr.substr(i, tempStr.length()).c_str()));

			cur->child[ind] = newNode->fileName;
			newNode->child[tempInd] = nextNode->fileName;

			writeRTree(newNode, newNode->fileName, RTree);
			writeRTree(nextNode, nextNode->fileName, RTree);
			writeRTree(cur, cur->fileName, RTree);

			delete newNode;
			delete nextNode;
		}

		readRTree(cur, cur->child[ind], RTree);
	}
}

//---------------------------

void query()
{
	string path = "D:\\Programming\\Results\\1000 queries.txt";

	ifstream fin;

	fin.open(path);

	string str;

	while (!fin.eof())
	{
		getline(fin, str, '\n');
		
		int* topFile = new int[20];

		int count = topNewsFile(str, topFile);

		cout << str << " " << count << endl;

		delete[] topFile;

		break;
	}


	fin.close();
}