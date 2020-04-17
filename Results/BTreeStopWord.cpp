#include "BTreeStopWord.h"

BNodeStopWord* readStrStopWord(int ind, fstream& BTree)
{
	if (!BTree.is_open())
		return NULL;

	int pos = ind * sizeof BNodeStopWord;

	BTree.seekg(pos, BTree.beg);

	BNodeStopWord* cur = new BNodeStopWord;

	BTree.read((char*)cur, sizeof BNodeStopWord);

	return cur;
}

void writeStrStopWord(BNodeStopWord* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTreeStopWord.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNodeStopWord;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNodeStopWord);
}

void splitStrStopWord(BNodeStopWord* cur, int ind, BNodeStopWord* indCur, int& curInd, fstream& BTree)
{
	BNodeStopWord* left = indCur;


	BNodeStopWord* right = new BNodeStopWord;
	right->index = curInd;
	right->isLeaf = left->isLeaf;
	right->n = t - 1;

	for (int i = 0; i < right->n; ++i)
	{
		right->key[i] = left->key[i + t];
	}

	if (!right->isLeaf)
		for (int i = 0; i < right->n + 1; ++i)
			right->child[i] = left->child[i + t];

	++curInd;


	for (int i = cur->n; i >= ind + 1; --i)
		cur->child[i + 1] = cur->child[i];

	cur->child[ind + 1] = right->index;

	for (int i = cur->n - 1; i >= ind; --i)
	{
		cur->key[i + 1] = cur->key[i];
	}

	cur->key[ind] = left->key[t - 1];

	++cur->n;


	for (int i = t - 1; i < left->n; ++i)
	{
		left->key[i].str[0] = '`';
	}

	if (!left->isLeaf)
		for (int i = t; i < left->n + 1; ++i)
			left->child[i] = -1;

	left->n = t - 1;


	writeStrStopWord(left, BTree);
	delete left;

	writeStrStopWord(right, BTree);
	delete right;
}

void insertNonFullStrStopWord(BNodeStopWord* cur, fixedStr k, int& curInd, fstream& BTree)
{
	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;


	if (cur->isLeaf)
	{
		int j = cur->n - 1;

		while (j > i)
		{
			cur->key[j + 1] = cur->key[j];
			--j;
		}

		cur->key[j + 1] = k;
		++cur->n;

		writeStrStopWord(cur, BTree);
		delete cur;
	}
	else
	{
		++i;

		BNodeStopWord* next = readStrStopWord(cur->child[i], BTree);

		if (next->n == 2 * t - 1)
		{
			splitStrStopWord(cur, i, next, curInd, BTree);

			writeStrStopWord(cur, BTree);

			if (k > cur->key[i])
				++i;

			next = readStrStopWord(cur->child[i], BTree);
		}

		delete cur;

		insertNonFullStrStopWord(next, k, curInd, BTree);
	}
}

void insertStrStopWord(int& rootInd, fixedStr k, int& curInd, fstream& BTree)
{
	BNodeStopWord* root = readStrStopWord(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNodeStopWord;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = k;

		writeStrStopWord(root, BTree);
		delete root;

		rootInd = curInd;
		++curInd;

		return;
	}

	if (root->n == 2 * t - 1)
	{
		BNodeStopWord* temp = root;
		root = new BNodeStopWord;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		splitStrStopWord(root, 0, temp, curInd, BTree);

		root->index = curInd;

		writeStrStopWord(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFullStrStopWord(root, k, curInd, BTree);
}

void prepareBTreeStopWord()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int rootInd, curInd;

	string metaFilePath = rootPath + "metaStopWord.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		rootInd = 0;
		curInd = 0;
	}
	else
	{
		metaRead >> rootInd >> curInd;
		metaRead.close();
	}

	fstream BTree;

	string BTreePath = rootPath + "BTreeStopWord.txt";

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string path = rootPathStopWord + stopWordFileName;

	ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		cout << "Can't open news file to read prepareBTreeStopWord" << endl; return;
	}

	fixedStr k;

	while (!fin.eof())
	{
		fin.get(k.str, 100, '\n');
		insertStrStopWord(rootInd, k, curInd, BTree);
		fin.ignore(200, '\n');
	}

	BTree.close();

	ofstream metaWrite;
	metaWrite.open(metaFilePath);
	if (!metaWrite.is_open())
	{
		SetConsoleTextAttribute(hConsole, 252);
		cout << "Can't open meta file to write";
		return;
	}

	metaWrite << rootInd << endl;
	metaWrite << curInd << endl;

	metaWrite.close();
}

bool strPosStopWord(int curInd, fixedStr k, fstream& BTree)
{
	BNodeStopWord* cur = readStrStopWord(curInd, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return true;

	if (cur->isLeaf)
		return false;
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return strPosStopWord(next, k, BTree);
	}
}

bool isStopWord(fixedStr k)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int rootInd;

	string metaFilePath = rootPath + "metaStopWord.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		SetConsoleTextAttribute(hConsole, 252);
		cout << "Can't open meta file to read." << endl;
		return false;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeStopWord.txt";

	fstream BTree;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);

	bool pos = strPosStopWord(rootInd, k, BTree);

	BTree.close();

	return pos;
}

void testStopWord(fixedStr k)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SetConsoleTextAttribute(hConsole, 252);
	if (isStopWord(k))
		cout << "YES, it is a stopword." << endl;
	else cout << "NO, it is NOT a stopword." << endl;
}