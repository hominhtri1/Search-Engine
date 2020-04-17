#include "Synonym.h"

void writeStrSyn(BNodeStr* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTreeSynonym.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNodeStr;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNodeStr);
}

void splitStrSyn(BNodeStr* cur, int ind, BNodeStr* indCur, int& curInd, fstream& BTree, int tellg)
{
	BNodeStr* left = indCur;


	BNodeStr* right = new BNodeStr;
	right->index = curInd;
	right->isLeaf = left->isLeaf;
	right->n = t - 1;

	for (int i = 0; i < right->n; ++i)
	{
		right->key[i] = left->key[i + t];
		right->value[i] = left->value[i + t];
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
		cur->value[i + 1] = cur->value[i];
	}

	cur->key[ind] = left->key[t - 1];
	cur->value[ind] = left->value[t - 1];

	++cur->n;


	for (int i = t - 1; i < left->n; ++i)
	{
		left->key[i].str[0] = '`';
		left->value[i] = -1;
	}

	if (!left->isLeaf)
		for (int i = t; i < left->n + 1; ++i)
			left->child[i] = -1;

	left->n = t - 1;


	writeStrSyn(left, BTree);
	delete left;

	writeStrSyn(right, BTree);
	delete right;
}

void insertNonFullStrSyn(BNodeStr* cur, fixedStr k, int& curInd, fstream& BTree, int tellg)
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
		cur->value[j + 1] = tellg;
		++cur->n;

		writeStrSyn(cur, BTree);
		delete cur;
	}
	else
	{
		++i;

		BNodeStr* next = readStr(cur->child[i], BTree);

		if (next->n == 2 * t - 1)
		{
			splitStrSyn(cur, i, next, curInd, BTree, tellg);

			writeStrSyn(cur, BTree);

			if (k > cur->key[i])
				++i;

			next = readStr(cur->child[i], BTree);
		}

		delete cur;

		insertNonFullStrSyn(next, k, curInd, BTree, tellg);
	}
}

void insertStrSyn(int& rootInd, fixedStr k, int& curInd, fstream& BTree, int tellg)
{
	BNodeStr* root = readStr(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNodeStr;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = k;
		root->value[0] = tellg;

		writeStrSyn(root, BTree);
		delete root;

		rootInd = curInd;
		++curInd;

		return;
	}

	if (root->n == 2 * t - 1)
	{
		BNodeStr* temp = root;
		root = new BNodeStr;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		splitStrSyn(root, 0, temp, curInd, BTree, tellg);

		root->index = curInd;

		writeStrSyn(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFullStrSyn(root, k, curInd, BTree, tellg);
}

void prepareBTreeSynonym()
{
	int rootInd, curInd;

	string metaFilePath = rootPath + "metaSynonym.txt";

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

	string BTreePath = rootPath + string("BTreeSynonym.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string path = rootPathSynonym + synonymFileName;

	ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		cout << "Can't open news file to read" << endl;
		return;
	}

	int curPos = 0;

	fixedStr k;

	while (!fin.eof())
	{

		curPos = fin.tellg();
		fin.get(k.str, 128, 124);

		int nextNLines;
		fin.ignore(5, 124);
		fin >> nextNLines;
		fin.ignore(5, 124);

		cout << curPos << " " << k.str << endl;
		insertStrSyn(rootInd, k, curInd, BTree, curPos);

		for (int i = 0; i < nextNLines; ++i)fin.ignore(50000, '\n');

	}

	BTree.close();
	fin.close();

	ofstream metaWrite;
	metaWrite.open(metaFilePath);
	if (!metaWrite.is_open())
	{
		cout << "Can't open meta file to write";
		return;
	}

	metaWrite << rootInd << endl;
	metaWrite << curInd << endl;

	metaWrite.close();
}

bool isDelimiterSyn(string text, int curPos)
{
	return (' ' == text[curPos] || '|' == text[curPos]);
}

SynonymsArray* getSynonyms(fixedStr k)
{
	int rootInd;

	string metaFilePath = rootPath + "metaSynonym.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeSynonym.txt";

	fstream BTree;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (-1 == pos)
	{
		cout << -1 << endl;
		return NULL;
	}

	ifstream SynonymFile;
	SynonymFile.open(rootPathSynonym + "synonym.txt");
	SynonymFile.seekg(pos, ios::beg);

	SynonymFile.ignore(500, 124);

	int lines;
	SynonymFile >> lines;

	SynonymFile.ignore(5, '\n');
	int startOfSynonym = SynonymFile.tellg();

	SynonymsArray *arr = new SynonymsArray;

	for (int i = 0; i < lines; ++i) {

		string text;
		getline(SynonymFile, text);
		int length = text.length();

		int curPos = 0;
		int tempPos;

		string word;

		while (curPos < length)
		{
			while (curPos < length && isDelimiterSyn(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && !isDelimiterSyn(text, tempPos + 1))
				++tempPos;

			word = text.substr(curPos, tempPos - curPos + 1);

			if (word != "") {
				fixedStr tmp;
				strcpy_s(tmp.str, word.c_str());
				arr->synonymsArray.push_back(tmp);
			}

			curPos = tempPos + 1;
		}
	}

	arr->wordCount = arr->synonymsArray.size();

	SynonymFile.close();
	BTree.close();

	return arr;
}

void testSynonym(fixedStr k)
{
	SynonymsArray*mResults = getSynonyms(k);

	if (mResults) {
		for (int i = 0; i < mResults->wordCount; ++i) {
			cout << mResults->synonymsArray[i].str << endl;
		}
		delete mResults;
	}
	else cout << "NOT FOUND " << k.str << endl;
}

void existInNewsFileBTreeSyn(fixedStr k, bool* exist)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read" << endl;
		return;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (pos != -1)
		fillExist(valueFile, pos, exist);

	SynonymsArray* mResults = getSynonyms(k);

	if (NULL == mResults)
	{
		BTree.close();
		valueFile.close();
		return;
	}

	for (int i = 0; i < mResults->wordCount; ++i)
	{
		int pos = strPos(rootInd, mResults->synonymsArray[i], BTree);

		if (pos != -1)
			fillExist(valueFile, pos, exist);
	}

	delete mResults;

	BTree.close();
	valueFile.close();
}

int* wordPositionsSyn(int newsFile, fixedStr k, int& curCount)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int* curPositions = NULL;

	int pos = strPos(rootInd, k, BTree);

	if (pos != -1)
		curPositions = positionsInFile(valueFile, pos, newsFile, curCount);

	SynonymsArray* mResults = getSynonyms(k);

	if (NULL == mResults)
	{
		BTree.close();
		valueFile.close();
		return curPositions;
	}

	for (int i = 0; i < mResults->wordCount; ++i)
	{
		int pos = strPos(rootInd, mResults->synonymsArray[i], BTree);

		if (-1 == pos)
			continue;

		int count = 0;

		int* positions = positionsInFile(valueFile, pos, newsFile, count);

		int* temp = curPositions;

		if (curCount + count > 0)
		{
			curPositions = new int[curCount + count];

			for (int i = 0; i < curCount; ++i)
				curPositions[i] = temp[i];

			for (int i = 0; i < count; ++i)
				curPositions[curCount + i] = positions[i];
		}

		curCount += count;

		if (temp != NULL)
			delete[] temp;

		if (positions != NULL)
			delete[] positions;
	}

	delete mResults;

	BTree.close();
	valueFile.close();

	sort(curPositions, curPositions + curCount);

	return curPositions;
}