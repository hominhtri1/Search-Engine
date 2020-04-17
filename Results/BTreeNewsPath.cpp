#include "BTreeNewsPath.h"


bool operator<(const fixedStrNewsPath & a, const fixedStrNewsPath & b)
{
	return (strcmp(a.str, b.str) < 0);
}

bool operator==(const fixedStrNewsPath & a, const fixedStrNewsPath & b)
{
	return (strcmp(a.str, b.str) == 0);
}

bool operator>(const fixedStrNewsPath & a, const fixedStrNewsPath & b)
{
	return (strcmp(a.str, b.str) > 0);
}

BNodeStrNewPath* readStrNewsPath(int ind, fstream& BTreeNewsPath)
{
	if (!BTreeNewsPath.is_open())
		return NULL;

	int pos = ind * sizeof BNodeStrNewPath;

	BTreeNewsPath.seekg(pos, BTreeNewsPath.beg);

	BNodeStrNewPath* cur = new BNodeStrNewPath;

	BTreeNewsPath.read((char*)cur, sizeof BNodeStrNewPath);

	return cur;
}

void writeStrNewsPath(BNodeStrNewPath* cur, fstream& BTreeNewsPath)
{
	if (!BTreeNewsPath.is_open())
	{
		string pathNewsPath = rootPath + string("BTreeNewsPath.txt");

		ofstream fout;
		fout.open(pathNewsPath);

		fout.close();

		BTreeNewsPath.open(pathNewsPath, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNodeStrNewPath;

	BTreeNewsPath.seekp(pos, BTreeNewsPath.beg);

	BTreeNewsPath.write((char*)cur, sizeof BNodeStrNewPath);
}

void splitStrNewsPath(BNodeStrNewPath* cur, int ind, BNodeStrNewPath* indCur, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath)
{
	BNodeStrNewPath* left = indCur;


	BNodeStrNewPath* right = new BNodeStrNewPath;
	right->index = curIndNewsPath;
	right->isLeaf = left->isLeaf;
	right->n = tNewsPath - 1;

	for (int i = 0; i < right->n; ++i)
	{
		right->key[i] = left->key[i + tNewsPath];
		right->value[i] = left->value[i + tNewsPath];
	}

	if (!right->isLeaf)
		for (int i = 0; i < right->n + 1; ++i)
			right->child[i] = left->child[i + tNewsPath];

	++curIndNewsPath;


	for (int i = cur->n; i >= ind + 1; --i)
		cur->child[i + 1] = cur->child[i];

	cur->child[ind + 1] = right->index;

	for (int i = cur->n - 1; i >= ind; --i)
	{
		cur->key[i + 1] = cur->key[i];
		cur->value[i + 1] = cur->value[i];
	}

	cur->key[ind] = left->key[tNewsPath - 1];
	cur->value[ind] = left->value[tNewsPath - 1];

	++cur->n;


	for (int i = tNewsPath - 1; i < left->n; ++i)
	{
		left->value[i].str[0] = '`';
		left->key[i] = -1;
	}

	if (!left->isLeaf)
		for (int i = tNewsPath; i < left->n + 1; ++i)
			left->child[i] = -1;

	left->n = tNewsPath - 1;


	writeStrNewsPath(left, BTreeNewsPath);
	delete left;

	writeStrNewsPath(right, BTreeNewsPath);
	delete right;
}

void insertNonFullStrNewsPath(BNodeStrNewPath* cur, int k, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath)
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
			cur->value[j + 1] = cur->value[j];
			--j;
		}

		cur->key[j + 1] = k;
		cur->value[j + 1] = newsPath;
		++cur->n;

		writeStrNewsPath(cur, BTreeNewsPath);
		delete cur;
	}
	else
	{
		++i;

		BNodeStrNewPath* next = readStrNewsPath(cur->child[i], BTreeNewsPath);

		if (next->n == 2 * tNewsPath - 1)
		{
			splitStrNewsPath(cur, i, next, curIndNewsPath, BTreeNewsPath, newsPath);

			writeStrNewsPath(cur, BTreeNewsPath);

			if (k > cur->key[i])
				++i;

			next = readStrNewsPath(cur->child[i], BTreeNewsPath);
		}

		delete cur;

		insertNonFullStrNewsPath(next, k, curIndNewsPath, BTreeNewsPath, newsPath);
	}
}

void insertStrNewsPath(int& rootIndNewsPath, int k, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath)
{
	BNodeStrNewPath* root = readStrNewsPath(rootIndNewsPath, BTreeNewsPath);

	if (NULL == root)
	{
		root = new BNodeStrNewPath;
		root->index = curIndNewsPath;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = k;
		root->value[0] = newsPath;

		writeStrNewsPath(root, BTreeNewsPath);
		delete root;

		rootIndNewsPath = curIndNewsPath;
		++curIndNewsPath;

		return;
	}

	if (root->n == 2 * tNewsPath - 1)
	{
		BNodeStrNewPath* temp = root;
		root = new BNodeStrNewPath;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		splitStrNewsPath(root, 0, temp, curIndNewsPath, BTreeNewsPath, newsPath);

		root->index = curIndNewsPath;

		writeStrNewsPath(root, BTreeNewsPath);

		rootIndNewsPath = curIndNewsPath;
		++curIndNewsPath;
	}

	insertNonFullStrNewsPath(root, k, curIndNewsPath, BTreeNewsPath, newsPath);
}

string strPosNewsPath(int curIndNewsPath, int k, fstream& BTreeNewsPath)
{
	BNodeStrNewPath* cur = readStrNewsPath(curIndNewsPath, BTreeNewsPath);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return cur->value[i].str;

	if (cur->isLeaf)
		return "";
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return strPosNewsPath(next, k, BTreeNewsPath);
	}
}

string getNewsPaths(int k)
{
	int rootIndNewsPath;

	string metaFilePathNewsPath = rootPath + "metaNewsPath.txt";

	ifstream metaReadNewsPath;
	metaReadNewsPath.open(metaFilePathNewsPath);
	if (!metaReadNewsPath.is_open())
	{
		cout << "Can't open meta file to read." << endl;
	}

	metaReadNewsPath >> rootIndNewsPath;

	metaReadNewsPath.close();

	string BTreePathNewsPath = rootPath + "BTreeNewsPath.txt";

	fstream BTreeNewsPath;

	BTreeNewsPath.open(BTreePathNewsPath, fstream::binary | fstream::out | fstream::in);

	string newsFilePathName = strPosNewsPath(rootIndNewsPath, k, BTreeNewsPath);

	if ("" == newsFilePathName)
	{
		cout << -1 << endl;
		return NULL;
	}

	return newsFilePathName;
}

void testNewsPath(int k)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	string mResult = getNewsPaths(k);

	if (mResult != "") {
		cout << mResult << endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 252);
		cout << endl << "NOT FOUND for " << k << endl;
	}
}