#include "BTree.h"
#include "BTreeNewsPath.h"
#include "BTreeStopWord.h"

bool operator<(const fixedStr & a, const fixedStr & b)
{
	return (strcmp(a.str, b.str) < 0);
}

bool operator==(const fixedStr & a, const fixedStr & b)
{
	return (strcmp(a.str, b.str) == 0);
}

bool operator>(const fixedStr & a, const fixedStr & b)
{
	return (strcmp(a.str, b.str) > 0);
}

BNode* read(int ind, fstream& BTree)
{
	if (!BTree.is_open())
		return NULL;

	int pos = ind * sizeof BNode;

	BTree.seekg(pos, BTree.beg);

	BNode* cur = new BNode;

	BTree.read((char*)cur, sizeof BNode);

	return cur;
}

BNodeStr* readStr(int ind, fstream& BTree)
{
	if (!BTree.is_open())
		return NULL;

	int pos = ind * sizeof BNodeStr;

	BTree.seekg(pos, BTree.beg);

	BNodeStr* cur = new BNodeStr;

	BTree.read((char*)cur, sizeof BNodeStr);

	return cur;
}

void write(BNode* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTreeNum.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNode;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNode);
}

void writeStr(BNodeStr* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTree.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNodeStr;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNodeStr);
}

int writeToValueFile(fstream& valueFile, int newsFile, int count, int firstPos)
{
	valueNode* tempNode = new valueNode;

	valueFile.seekp(0, valueFile.end);
	int endPos = valueFile.tellp();
	tempNode->pos = endPos;
	tempNode->fileInd = newsFile;
	tempNode->count = count;
	tempNode->curCount = 1;
	valueFile.write((char*)tempNode, sizeof valueNode);

	delete tempNode;

	int temp;

	int* memblock = new int[count + 1];
	memblock[0] = firstPos;
	memblock[count] = -1;

	valueFile.seekp(0, valueFile.end);
	valueFile.write((char*)memblock, (count + 1) * (sizeof temp));

	delete[] memblock;

	return endPos;
}

void updateValueFile(fstream& valueFile, int pos, int newsFile, int count, int wordPos)
{
	valueNode* curNode = new valueNode;

	int curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	int temp;

	int* memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	int next = memblock[curNode->count];

	while (curNode->fileInd != newsFile && next != -1)
	{
		curPos = next;
		valueFile.seekg(curPos, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		next = memblock[curNode->count];
	}

	if (curNode->fileInd != newsFile)
	{
		valueFile.seekg(0, valueFile.end);
		int endPos = valueFile.tellg();
		memblock[curNode->count] = endPos;

		valueFile.seekp(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.write((char*)memblock, (curNode->count + 1) * (sizeof temp));

		curPos = endPos;

		curNode->pos = curPos;
		curNode->fileInd = newsFile;
		curNode->count = count;
		curNode->curCount = 0;
	}

	delete[] memblock;

	memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	memblock[curNode->curCount] = wordPos;
	memblock[curNode->count] = -1;

	valueFile.seekp(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.write((char*)memblock, (curNode->count + 1) * (sizeof temp));

	delete[] memblock;

	++curNode->curCount;

	valueFile.seekp(curNode->pos, valueFile.beg);
	valueFile.write((char*)curNode, sizeof valueNode);

	delete curNode;
}

int writeToTempFile(fstream& tempFile)
{
	tempNode* temp = new tempNode;

	tempFile.seekp(0, tempFile.end);
	int endPos = tempFile.tellp();
	temp->pos = endPos;
	temp->count = 1;
	tempFile.write((char*)temp, sizeof tempNode);

	delete temp;

	return endPos;
}

void updateTempFile(fstream& tempFile, int pos)
{
	tempNode* temp = new tempNode;

	tempFile.seekg(pos, tempFile.beg);
	tempFile.read((char*)temp, sizeof tempNode);

	++temp->count;

	tempFile.seekp(pos, tempFile.beg);
	tempFile.write((char*)temp, sizeof tempNode);

	delete temp;
}

void split(BNode* cur, int ind, BNode* indCur, int& curInd, fstream& BTree)
{
	BNode* left = indCur;


	BNode* right = new BNode;
	right->index = curInd;
	right->isLeaf = left->isLeaf;
	right->n = t - 1;

	for (int i = 0; i < right->n; ++i)
	{
		right->key[i] = left->key[i + t];
		right->tempValue[i] = left->tempValue[i + t];
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
		cur->tempValue[i + 1] = cur->tempValue[i];
		cur->value[i + 1] = cur->value[i];
	}

	cur->key[ind] = left->key[t - 1];
	cur->tempValue[ind] = left->tempValue[t - 1];
	cur->value[ind] = left->value[t - 1];

	++cur->n;


	for (int i = t - 1; i < left->n; ++i)
	{
		left->key[i] = -1;
		left->tempValue[i] = -1;
		left->value[i] = -1;
	}

	if (!left->isLeaf)
		for (int i = t; i < left->n + 1; ++i)
			left->child[i] = -1;

	left->n = t - 1;


	write(left, BTree);
	delete left;

	write(right, BTree);
	delete right;
}

void splitStr(BNodeStr* cur, int ind, BNodeStr* indCur, int& curInd, fstream& BTree)
{
	BNodeStr* left = indCur;


	BNodeStr* right = new BNodeStr;
	right->index = curInd;
	right->isLeaf = left->isLeaf;
	right->n = t - 1;

	for (int i = 0; i < right->n; ++i)
	{
		right->key[i] = left->key[i + t];
		right->tempValue[i] = left->tempValue[i + t];
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
		cur->tempValue[i + 1] = cur->tempValue[i];
		cur->value[i + 1] = cur->value[i];
	}

	cur->key[ind] = left->key[t - 1];
	cur->tempValue[ind] = left->tempValue[t - 1];
	cur->value[ind] = left->value[t - 1];

	++cur->n;


	for (int i = t - 1; i < left->n; ++i)
	{
		left->key[i].str[0] = '`';
		left->tempValue[i] = -1;
		left->value[i] = -1;
	}

	if (!left->isLeaf)
		for (int i = t; i < left->n + 1; ++i)
			left->child[i] = -1;

	left->n = t - 1;


	writeStr(left, BTree);
	delete left;

	writeStr(right, BTree);
	delete right;
}

void insertNonFull(BNode* cur, int k, int& curInd, fstream& BTree, fstream& tempFile)
{
	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
	{
		if (-1 == cur->tempValue[i])
		{
			cur->tempValue[i] = writeToTempFile(tempFile);
			write(cur, BTree);
		}
		else
			updateTempFile(tempFile, cur->tempValue[i]);

		delete cur;

		return;
	}

	if (cur->isLeaf)
	{
		int j = cur->n - 1;

		while (j > i)
		{
			cur->key[j + 1] = cur->key[j];
			cur->tempValue[j + 1] = cur->tempValue[j];
			cur->value[j + 1] = cur->value[j];
			--j;
		}

		cur->key[j + 1] = k;
		cur->tempValue[j + 1] = writeToTempFile(tempFile);
		cur->value[j + 1] = -1;
		++cur->n;

		write(cur, BTree);
		delete cur;
	}
	else
	{
		++i;

		BNode* next = read(cur->child[i], BTree);

		if (next->n == 2 * t - 1)
		{
			split(cur, i, next, curInd, BTree);

			write(cur, BTree);

			if (k == cur->key[i])
			{
				if (-1 == cur->tempValue[i])
				{
					cur->tempValue[i] = writeToTempFile(tempFile);
					write(cur, BTree);
				}
				else
					updateTempFile(tempFile, cur->tempValue[i]);

				delete cur;

				return;
			}

			if (k > cur->key[i])
				++i;

			next = read(cur->child[i], BTree);
		}

		delete cur;

		insertNonFull(next, k, curInd, BTree, tempFile);
	}
}

void insertNonFullStr(BNodeStr* cur, fixedStr k, int& curInd, fstream& BTree, fstream& tempFile)
{
	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
	{
		if (-1 == cur->tempValue[i])
		{
			cur->tempValue[i] = writeToTempFile(tempFile);
			writeStr(cur, BTree);
		}
		else
			updateTempFile(tempFile, cur->tempValue[i]);

		delete cur;

		return;
	}

	if (cur->isLeaf)
	{
		int j = cur->n - 1;

		while (j > i)
		{
			cur->key[j + 1] = cur->key[j];
			cur->tempValue[j + 1] = cur->tempValue[j];
			cur->value[j + 1] = cur->value[j];
			--j;
		}

		cur->key[j + 1] = k;
		cur->tempValue[j + 1] = writeToTempFile(tempFile);
		cur->value[j + 1] = -1;
		++cur->n;

		writeStr(cur, BTree);
		delete cur;
	}
	else
	{
		++i;

		BNodeStr* next = readStr(cur->child[i], BTree);

		if (next->n == 2 * t - 1)
		{
			splitStr(cur, i, next, curInd, BTree);

			writeStr(cur, BTree);

			if (k == cur->key[i])
			{
				if (-1 == cur->tempValue[i])
				{
					cur->tempValue[i] = writeToTempFile(tempFile);
					writeStr(cur, BTree);
				}
				else
					updateTempFile(tempFile, cur->tempValue[i]);

				delete cur;

				return;
			}

			if (k > cur->key[i])
				++i;

			next = readStr(cur->child[i], BTree);
		}

		delete cur;

		insertNonFullStr(next, k, curInd, BTree, tempFile);
	}
}

void insert(int& rootInd, int k, int& curInd, fstream& BTree, fstream& tempFile)
{
	BNode* root = read(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNode;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = k;
		root->tempValue[0] = writeToTempFile(tempFile);
		root->value[0] = -1;

		write(root, BTree);
		delete root;

		rootInd = curInd;
		++curInd;

		return;
	}

	if (root->n == 2 * t - 1)
	{
		BNode* temp = root;
		root = new BNode;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		split(root, 0, temp, curInd, BTree);

		root->index = curInd;

		write(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFull(root, k, curInd, BTree, tempFile);
}

void insertStr(int& rootInd, fixedStr k, int& curInd, fstream& BTree, fstream& tempFile)
{
	BNodeStr* root = readStr(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNodeStr;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = k;
		root->tempValue[0] = writeToTempFile(tempFile);
		root->value[0] = -1;

		writeStr(root, BTree);
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

		splitStr(root, 0, temp, curInd, BTree);

		root->index = curInd;

		writeStr(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFullStr(root, k, curInd, BTree, tempFile);
}

void fillExist(fstream& valueFile, int pos, bool* exist)
{
	valueNode* curNode = new valueNode;

	int curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	exist[curNode->fileInd] = true;

	int temp;

	int* memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	int next = memblock[curNode->count];

	while (next != -1)
	{
		curPos = next;
		valueFile.seekg(curPos, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		exist[curNode->fileInd] = true;

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		next = memblock[curNode->count];
	}

	delete curNode;
}

void range(int curInd, fstream& BTree, fstream& valueFile, int start, int end, bool* exist)
{
	BNode* cur = read(curInd, BTree);

	for (int i = 0; i < cur->n; ++i)
	{
		if (cur->key[i] >= start && cur->key[i] <= end)
			fillExist(valueFile, cur->value[i], exist);
	}

	if (cur->isLeaf != true)
	{
		if (start < cur->key[0])
			range(cur->child[0], BTree, valueFile, start, end, exist);

		for (int i = 1; i < cur->n; ++i)
		{
			if (end <= cur->key[i - 1] || start >= cur->key[i])
				continue;

			range(cur->child[i], BTree, valueFile, start, end, exist);
		}

		if (end > cur->key[cur->n - 1])
			range(cur->child[cur->n], BTree, valueFile, start, end, exist);
	}

	delete cur;
}

void countRange(int curInd, fstream& BTree, int start, int end, int& count)
{
	BNode* cur = read(curInd, BTree);

	for (int i = 0; i < cur->n; ++i)
	{
		if (cur->key[i] >= start && cur->key[i] <= end)
			++count;
	}

	if (cur->isLeaf != true)
	{
		if (start < cur->key[0])
			countRange(cur->child[0], BTree, start, end, count);

		for (int i = 1; i < cur->n; ++i)
		{
			if (end <= cur->key[i - 1] || start >= cur->key[i])
				continue;

			countRange(cur->child[i], BTree, start, end, count);
		}

		if (end > cur->key[cur->n - 1])
			countRange(cur->child[cur->n], BTree, start, end, count);
	}

	delete cur;
}

void updateRangePositions(int curInd, int newsFile, fstream& BTree, fstream& valueFile, int start, int end, int* &curPositions, int& curCount)
{
	BNode* cur = read(curInd, BTree);

	for (int i = 0; i < cur->n; ++i)
	{
		if (cur->key[i] >= start && cur->key[i] <= end)
		{
			int pos = cur->value[i];

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
	}

	if (cur->isLeaf != true)
	{
		if (start < cur->key[0])
			updateRangePositions(cur->child[0], newsFile, BTree, valueFile, start, end, curPositions, curCount);

		for (int i = 1; i < cur->n; ++i)
		{
			if (end <= cur->key[i - 1] || start >= cur->key[i])
				continue;

			updateRangePositions(cur->child[i], newsFile, BTree, valueFile, start, end, curPositions, curCount);
		}

		if (end > cur->key[cur->n - 1])
			updateRangePositions(cur->child[cur->n], newsFile, BTree, valueFile, start, end, curPositions, curCount);
	}

	delete cur;
}

bool isNumber(string text, int curPos)
{
	return (text[curPos] >= 48 && text[curPos] <= 57);
}

bool isDelimiter(string text, int curPos)
{
	return(text[curPos] == ' ' || text[curPos] == 10 || text[curPos] == '|' || text[curPos] == ',' 
		|| text[curPos] == ':' || text[curPos] == '!' || text[curPos] == '.' || text[curPos] == ' ' 
		|| text[curPos] == '"' || text[curPos] == '\n' || text[curPos] == '-'
		|| text[curPos] == '(' || text[curPos] == '—' || text[curPos] == ')' || text[curPos] == '/'
		);
}

void generateRandFiles(int numFiles, int fileSize, int numRange)
{
	for (int i = 0; i < numFiles; ++i)
	{
		string path = rootPath + "Rand files\\" + to_string(i) + "rand.txt";

		ofstream fout;
		fout.open(path);
		if (!fout.is_open())
		{
			cout << "Can't open rand file to write" << endl;
			return;
		}

		for (int j = 0; j < fileSize; ++j)
			fout << (rand() % numRange) << endl;

		fout.close();
	}
}

void prepareBTreeData(int startFile, int endFile)
{
	int rootInd, curInd;

	string metaFilePath = rootPath + "metaNum.txt";

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

	fstream BTree, valueFile, tempFile;

	string BTreePath = rootPath + string("BTreeNum.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string valuePath = rootPath + "valueNum.txt";

	// Make sure the file is there before opening with valueFile
	ofstream tempFout;
	tempFout.open(valuePath, ios::app);

	tempFout.close();

	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	for (int i = startFile; i <= endFile; ++i)
	{
	
		string tempPath = rootPath + "tempNum.txt";

		tempFout.open(tempPath);

		tempFout.close();

		tempFile.open(tempPath, fstream::binary | fstream::out | fstream::in);

		string path = getNewsPaths(i);

		ifstream fin;
		fin.open(path);
		if (!fin.is_open())
		{
			cout << "Can't open news file to read prepareBTreeData" << endl;
			return;
		}

		stringstream buffer;
		buffer << fin.rdbuf();

		fin.close();

		string text = buffer.str();
		int length = text.length();

		int curPos = 0;
		int tempPos;

		string numStr;

		int num;

		while (curPos < length)
		{
			while (curPos < length && !isNumber(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && isNumber(text, tempPos + 1))
				++tempPos;

			numStr = text.substr(curPos, tempPos - curPos + 1);

			num = stoi(numStr, NULL);

			insert(rootInd, num, curInd, BTree, tempFile);

			curPos = tempPos + 1;
		}

		curPos = 0;

		while (curPos < length)
		{
			while (curPos < length && !isNumber(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && isNumber(text, tempPos + 1))
				++tempPos;

			numStr = text.substr(curPos, tempPos - curPos + 1);

			num = stoi(numStr, NULL);

			updateValue(rootInd, num, curPos, i, BTree, valueFile, tempFile);

			curPos = tempPos + 1;
		}

		tempFile.close();
	}

	BTree.close();
	valueFile.close();

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

void prepareBTreeDataStr2(int startFile, int endFile)
{

	// Haven't optimized stopword, look at prepareBTreeDataStr()

	int rootInd, curInd;

	string metaFilePath = rootPath + "meta.txt";

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

	fstream BTree, valueFile, tempFile;

	string BTreePath = rootPath + string("BTree.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string valuePath = rootPath + "value.txt";

	// Make sure the file is there before opening with valueFile
	ofstream tempFout;
	tempFout.open(valuePath, ios::app);

	tempFout.close();

	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	for (int i = startFile; i <= endFile; ++i)
	{
		//cout << "News " << i << endl;

		string tempPath = rootPath + "temp.txt";

		tempFout.open(tempPath);

		tempFout.close();

		tempFile.open(tempPath, fstream::binary | fstream::out | fstream::in);

		string path = rootPath + "News files\\Group05News" + to_string(i) + ".txt";

		ifstream fin;
		fin.open(path);
		if (!fin.is_open())
		{
			cout << "Can't open news file to read prepareBTreeDataStr2" << endl;
			return;
		}

		stringstream buffer;
		buffer << fin.rdbuf();

		fin.close();

		string text = buffer.str();
		int length = text.length();

		int curPos = 0;
		int tempPos;

		string word;

		fixedStr k;

		while (curPos < length)
		{
			while (curPos < length && isDelimiter(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && !isDelimiter(text, tempPos + 1))
				++tempPos;

			word = text.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, word.c_str());

			if (!isStopWord(k))
				insertStr(rootInd, k, curInd, BTree, tempFile);

			curPos = tempPos + 1;
		}

		curPos = 0;

		while (curPos < length)
		{
			while (curPos < length && isDelimiter(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && !isDelimiter(text, tempPos + 1))
				++tempPos;

			word = text.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, word.c_str());

			if (!isStopWord(k))
				updateValueStr(rootInd, k, curPos, i, BTree, valueFile, tempFile);

			curPos = tempPos + 1;
		}

		tempFile.close();
	}

	BTree.close();
	valueFile.close();

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

void prepareBTreeDataStr(int& startFile, int& endFile, int index)
{
	int rootIndSW;

	string metaFilePathSW = rootPath + "metaStopWord.txt";

	ifstream metaReadSW;
	metaReadSW.open(metaFilePathSW);
	if (!metaReadSW.is_open())
	{
		cout << "Can't open meta file to read prepareBTreeDataStr" << endl;
		return;
	}

	metaReadSW >> rootIndSW;

	metaReadSW.close();

	string BTreePathSW = rootPath + "BTreeStopWord.txt";

	fstream BTreeSW;

	BTreeSW.open(BTreePathSW, fstream::binary | fstream::out | fstream::in);

	//----------------------------------------------------------------

	/*START Section of other btree for btree_news_pathNewsPath*/
	int rootIndNewsPath, curIndNewsPath, newsPathCurInd;

	string metaFilePathNewsPath = rootPath + "metaNewsPath.txt";

	ifstream metaReadNewsPath;
	metaReadNewsPath.open(metaFilePathNewsPath);
	if (!metaReadNewsPath.is_open())
	{
		rootIndNewsPath = 0;
		curIndNewsPath = 0;
		newsPathCurInd = -1;
	}
	else
	{
		metaReadNewsPath >> rootIndNewsPath >> curIndNewsPath >> newsPathCurInd;
		metaReadNewsPath.close();
	}

	fstream BTreeNewsPath;

	string BTreePathNewsPath = rootPath + string("BTreeNewsPath.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTreeNewsPath. If it isn't, do nothing
	ifstream tempFinNewsPath;
	tempFinNewsPath.open(BTreePathNewsPath);
	if (tempFinNewsPath.is_open())
	{
		tempFinNewsPath.close();

		BTreeNewsPath.open(BTreePathNewsPath, fstream::binary | fstream::out | fstream::in);
	}

	/*END Section of other btree for btree_news_pathNewsPath*/

	int rootInd, curInd;

	string metaFilePath = rootPath + "meta.txt";

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

	fstream BTree, valueFile, tempFile;

	string BTreePath = rootPath + string("BTree.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string valuePath = rootPath + "value.txt";

	// Make sure the file is there before opening with valueFile
	ofstream tempFout;
	tempFout.open(valuePath, ios::app);

	tempFout.close();

	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	std::string path = rootPath + rootNewsFolder + to_string(index);

	bool firstFile = true;

	for (const auto& pathNewsPath : fs::directory_iterator(path)) {

		cout << pathNewsPath.path() << endl;
		string tempPath = rootPath + "temp.txt";

		tempFout.open(tempPath);

		tempFout.close();

		tempFile.open(tempPath, fstream::binary | fstream::out | fstream::in);

		ifstream fin;
		fin.open(pathNewsPath);
		if (!fin.is_open())
		{
			cout << "Can't open news file to read 2" << endl;
			return;
		}

		/*START Section of other btree for btree_news_pathNewsPath*/
		++newsPathCurInd;

		if (firstFile)
		{
			startFile = newsPathCurInd;
			firstFile = false;
		}
		endFile = newsPathCurInd;

		fixedStrNewsPath tmpNewsPath;
		strcpy_s(tmpNewsPath.str, strNewsPathSize, pathNewsPath.path().string().c_str());
		insertStrNewsPath(rootIndNewsPath, newsPathCurInd, curIndNewsPath, BTreeNewsPath, tmpNewsPath);
		/*END Section of other btree for btree_news_pathNewsPath*/


		stringstream buffer;
		buffer << fin.rdbuf();

		fin.close();

		string text = buffer.str();
		int length = text.length();

		int curPos = 0;
		int tempPos;

		string word;

		fixedStr k;

		while (curPos < length)
		{
			while (curPos < length && isDelimiter(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && !isDelimiter(text, tempPos + 1))
				++tempPos;

			word = text.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, strSize, word.c_str());

			if (!strPosStopWord(rootIndSW, k, BTreeSW))
				insertStr(rootInd, k, curInd, BTree, tempFile);

			curPos = tempPos + 1;
		}

		curPos = 0;

		while (curPos < length)
		{
			while (curPos < length && isDelimiter(text, curPos))
				++curPos;

			if (curPos == length)
				break;

			tempPos = curPos;

			while (tempPos < length - 1 && !isDelimiter(text, tempPos + 1))
				++tempPos;

			word = text.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, strSize, word.c_str());

			if (!strPosStopWord(rootIndSW, k, BTreeSW))
				updateValueStr(rootInd, k, curPos, newsPathCurInd, BTree, valueFile, tempFile);

			curPos = tempPos + 1;
		}
		buffer.clear();
		tempFile.close();
	}

	BTree.close();
	valueFile.close();

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


	/*START Section of other btree for btree_news_pathNewsPath*/
	ofstream metaWriteNewsPath;
	metaWriteNewsPath.open(metaFilePathNewsPath);
	if (!metaWriteNewsPath.is_open())
	{
		cout << "Can't open meta file to write";
		return;
	}

	metaWriteNewsPath << rootIndNewsPath << endl;
	metaWriteNewsPath << curIndNewsPath << endl;
	metaWriteNewsPath << newsPathCurInd << endl;

	metaWriteNewsPath.close();
	/*END Section of other btree for btree_news_pathNewsPath*/

	//----------------------------------------------------------------------

	BTreeSW.close();
}

void rangeExist(int startNum, int endNum, bool* exist)
{
	int rootInd;

	string metaFilePath = rootPath + "metaNum.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read rangeExist" << endl;
		return;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeNum.txt";
	string valuePath = rootPath + "valueNum.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	range(rootInd, BTree, valueFile, startNum, endNum, exist);

	BTree.close();
	valueFile.close();
}

int numPos(int curInd, int k, fstream& BTree)
{
	BNode* cur = read(curInd, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return cur->value[i];

	if (cur->isLeaf)
		return -1;
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return numPos(next, k, BTree);
	}
}

int strPos(int curInd, fixedStr k, fstream& BTree)
{
	BNodeStr* cur = readStr(curInd, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return cur->value[i];

	if (cur->isLeaf)
		return -1;
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return strPos(next, k, BTree);
	}
}

BNode* containingNode(int curInd, int k, fstream& BTree)
{
	BNode* cur = read(curInd, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return cur;

	if (cur->isLeaf)
		return NULL;
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return containingNode(next, k, BTree);
	}
}

BNodeStr* containingNodeStr(int curInd, fixedStr k, fstream& BTree)
{
	BNodeStr* cur = readStr(curInd, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	if (i >= 0 && k == cur->key[i])
		return cur;

	if (cur->isLeaf)
		return NULL;
	else
	{
		int next = cur->child[i + 1];

		delete cur;

		return containingNodeStr(next, k, BTree);
	}
}

void updateValue(int rootInd, int k, int kPos, int curNewsFile, fstream& BTree, fstream& valueFile, fstream& tempFile)
{
	BNode* cur = containingNode(rootInd, k, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	int count = -1;

	if (cur->tempValue[i] != -1)
	{
		tempNode* temp = new tempNode;

		tempFile.seekg(cur->tempValue[i], tempFile.beg);
		tempFile.read((char*)temp, sizeof tempNode);

		count = temp->count;

		delete temp;

		cur->tempValue[i] = -1;
		write(cur, BTree);
	}

	if (-1 == cur->value[i])
	{
		cur->value[i] = writeToValueFile(valueFile, curNewsFile, count, kPos);
		write(cur, BTree);
	}
	else
		updateValueFile(valueFile, cur->value[i], curNewsFile, count, kPos);

	delete cur;
}

void updateValueStr(int rootInd, fixedStr k, int kPos, int curNewsFile, fstream& BTree, fstream& valueFile, fstream& tempFile)
{
	BNodeStr* cur = containingNodeStr(rootInd, k, BTree);

	int i = cur->n - 1;

	while (i >= 0 && k < cur->key[i])
		--i;

	int count = -1;

	if (cur->tempValue[i] != -1)
	{
		tempNode* temp = new tempNode;

		tempFile.seekg(cur->tempValue[i], tempFile.beg);
		tempFile.read((char*)temp, sizeof tempNode);

		count = temp->count;

		delete temp;

		cur->tempValue[i] = -1;
		writeStr(cur, BTree);
	}

	if (-1 == cur->value[i])
	{
		cur->value[i] = writeToValueFile(valueFile, curNewsFile, count, kPos);
		writeStr(cur, BTree);
	}
	else
		updateValueFile(valueFile, cur->value[i], curNewsFile, count, kPos);

	delete cur;
}

int* positionsInFile(fstream& valueFile, int pos, int newsFile, int& count)
{
	valueNode* curNode = new valueNode;

	int curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	int temp;

	int* memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	int next = memblock[curNode->count];

	while (curNode->fileInd != newsFile && next != -1)
	{
		curPos = next;
		valueFile.seekg(curPos, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		next = memblock[curNode->count];
	}

	if (curNode->fileInd != newsFile)
		return NULL;

	count = curNode->count;

	int* output = new int[count];

	for (int i = 0; i < count; ++i)
		output[i] = memblock[i];

	delete curNode;
	delete[] memblock;

	return output;
}

int countInFile(fstream& valueFile, int pos, int newsFile)
{
	valueNode* curNode = new valueNode;

	int curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	int temp;

	int* memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	int next = memblock[curNode->count];

	while (curNode->fileInd != newsFile && next != -1)
	{
		curPos = next;
		valueFile.seekg(curPos, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		next = memblock[curNode->count];
	}

	if (curNode->fileInd != newsFile)
		return 0;

	int count = curNode->count;

	delete curNode;
	delete[] memblock;

	return count;
}

void existInNewsFileBTree(fixedStr k, bool* exist)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read NewsFileBTree" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (-1 == pos)
		return;

	fillExist(valueFile, pos, exist);

	BTree.close();
	valueFile.close();
}

int* wordPositions(int newsFile, fixedStr k, int& count)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read wordPositions" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (-1 == pos)
		return NULL;

	int* positions = positionsInFile(valueFile, pos, newsFile, count);

	BTree.close();
	valueFile.close();

	return positions;
}

void printPositions(int newsFile, fixedStr k)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read printPositions" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (-1 == pos)
	{
		cout << -1 << endl;
	}

	int count = 0;

	int* positions = positionsInFile(valueFile, pos, newsFile, count);

	cout << count << endl;

	for (int i = 0; i < count; ++i)
		cout << positions[i] << " ";

	cout << endl;

	BTree.close();
	valueFile.close();

	delete[] positions;
}

int* rangePositions(int newsFile, int start, int end, int& count)
{
	int rootInd;

	string metaFilePath = rootPath + "metaNum.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read rangePositions" << endl;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeNum.txt";
	string valuePath = rootPath + "valueNum.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int* positions;
	count = 0;

	updateRangePositions(rootInd, newsFile, BTree, valueFile, start, end, positions, count);

	sort(positions, positions + count);

	BTree.close();
	valueFile.close();

	return positions;
}

//-------------------------------------------------------------------------------------------------

SearchResultArray* searchFor(fixedStr k)
{
	int rootInd;

	string metaFilePath = rootPath + "meta.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read searchFor" << endl;
		return NULL;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTree.txt";
	string valuePath = rootPath + "value.txt";

	fstream BTree, valueFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	valueFile.open(valuePath, fstream::binary | fstream::out | fstream::in);

	int pos = strPos(rootInd, k, BTree);

	if (-1 == pos)
	{
		cout << -1 << endl;
		return NULL;
	}

	SearchResultArray *arr = NULL;

	valueNode* curNode = new valueNode;

	int curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	int temp;

	int* memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	int next = memblock[curNode->count];
	int firstIteration = next;

	int filesCount = 1;

	while (firstIteration != -1)
	{
		curPos = firstIteration;
		valueFile.seekg(firstIteration, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		firstIteration = memblock[curNode->count];

		++filesCount;
	}

	arr = new SearchResultArray;
	arr->filesCount = filesCount;
	arr->searchResultArray = new SearchResult[filesCount];
	int i = 0;

	curPos = pos;
	valueFile.seekg(curPos, valueFile.beg);
	valueFile.read((char*)curNode, sizeof valueNode);
	valueFile.clear();

	delete[] memblock;

	memblock = new int[curNode->count + 1];

	valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
	valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
	valueFile.clear();

	arr->searchResultArray[i].fileInd = curNode->fileInd;
	arr->searchResultArray[i].count = curNode->count;
	arr->searchResultArray[i].positions = new int[arr->searchResultArray[i].count];
	for (int j = 0; j < arr->searchResultArray[i].count; ++j)
		arr->searchResultArray[i].positions[j] = memblock[j];

	++i;

	while (next != -1 && i < filesCount)
	{
		curPos = next;
		valueFile.seekg(curPos, valueFile.beg);
		valueFile.read((char*)curNode, sizeof valueNode);
		valueFile.clear();

		arr->searchResultArray[i].fileInd = curNode->fileInd;

		delete[] memblock;

		memblock = new int[curNode->count + 1];

		valueFile.seekg(curPos + (sizeof valueNode), valueFile.beg);
		valueFile.read((char*)memblock, (curNode->count + 1) * (sizeof temp));
		valueFile.clear();

		arr->searchResultArray[i].count = curNode->count;
		arr->searchResultArray[i].positions = new int[arr->searchResultArray[i].count];
		for (int j = 0; j < arr->searchResultArray[i].count; ++j)
			arr->searchResultArray[i].positions[j] = memblock[j];

		next = memblock[curNode->count];

		++i;
	}

	delete curNode;
	delete[] memblock;

	BTree.close();
	valueFile.close();

	return arr;
}

void printPositions(fixedStr k)
{
	SearchResultArray*mResults = searchFor(k);

	if (mResults) {
		for (int i = 0; i < mResults->filesCount; ++i) {
			cout << mResults->searchResultArray[i].fileInd << endl;
			cout << mResults->searchResultArray[i].count << endl;
			for (int j = 0; j < mResults->searchResultArray[i].count; ++j)
				cout << mResults->searchResultArray[i].positions[j] << " ";
			cout << endl << endl;
		}
		delete mResults;
	}
	else cout << "NOT FOUND " << k.str << endl;
}

void existInNewsFileTitleBTree(fixedStr k, bool* exist)
{
	SearchResultArray* mResults = searchFor(k);

	if (!mResults)
		return;

	int rootInd;

	string metaFilePath = rootPath + "metaPara.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read existInNewsFileTitleBTree" << endl;
		return;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeParagraph.txt";
	string paraPath = rootPath + "para.txt";

	fstream BTree, paraPosFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	paraPosFile.open(paraPath, fstream::binary | fstream::out | fstream::in);

	for (int i = 0; i < mResults->filesCount; ++i)
	{
		int curNewsFile = (mResults->searchResultArray[i]).fileInd;
		int curFirstPos = (mResults->searchResultArray[i]).positions[0];

		int pos = numPos(rootInd, curNewsFile, BTree);
		int paraEnd = firstParagraphEnding(paraPosFile, pos, curNewsFile);

		if (curFirstPos <= paraEnd)
			exist[curNewsFile] = true;
	}

	BTree.close();
	paraPosFile.close();

	delete mResults;
}

//-------------------------------------------------------------------------------------------------

void writePara(BNode* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTreeParagraph.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNode;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNode);
}

void writeWord(BNode* cur, fstream& BTree)
{
	if (!BTree.is_open())
	{
		string path = rootPath + string("BTreeWord.txt");

		ofstream fout;
		fout.open(path);

		fout.close();

		BTree.open(path, fstream::binary | fstream::out | fstream::in);
	}

	int pos = cur->index * sizeof BNode;

	BTree.seekp(pos, BTree.beg);

	BTree.write((char*)cur, sizeof BNode);
}

void insertNonFullParaWord(BNode* cur, int newsFile, int pos, int& curInd, fstream& BTree)
{
	int i = cur->n - 1;

	while (i >= 0 && newsFile < cur->key[i])
		--i;

	if (i >= 0 && newsFile == cur->key[i])
	{
		delete cur;

		return;
	}

	if (cur->isLeaf)
	{
		int j = cur->n - 1;

		while (j > i)
		{
			cur->key[j + 1] = cur->key[j];
			cur->tempValue[j + 1] = cur->tempValue[j];
			cur->value[j + 1] = cur->value[j];
			--j;
		}

		cur->key[j + 1] = newsFile;
		cur->value[j + 1] = pos;
		++cur->n;

		write(cur, BTree);
		delete cur;
	}
	else
	{
		++i;

		BNode* next = read(cur->child[i], BTree);

		if (next->n == 2 * t - 1)
		{
			split(cur, i, next, curInd, BTree);

			write(cur, BTree);

			if (newsFile == cur->key[i])
			{
				delete cur;

				return;
			}

			if (newsFile > cur->key[i])
				++i;

			next = read(cur->child[i], BTree);
		}

		delete cur;

		insertNonFullParaWord(next, newsFile, pos, curInd, BTree);
	}
}

void insertPara(int& rootInd, int newsFile, int pos, int& curInd, fstream& BTree)
{
	BNode* root = read(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNode;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = newsFile;
		root->value[0] = pos;

		writePara(root, BTree);
		delete root;

		rootInd = curInd;
		++curInd;

		return;
	}

	if (root->n == 2 * t - 1)
	{
		BNode* temp = root;
		root = new BNode;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		split(root, 0, temp, curInd, BTree);

		root->index = curInd;

		write(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFullParaWord(root, newsFile, pos, curInd, BTree);
}

void insertWord(int& rootInd, int newsFile, int pos, int& curInd, fstream& BTree)
{
	BNode* root = read(rootInd, BTree);

	if (NULL == root)
	{
		root = new BNode;
		root->index = curInd;
		root->isLeaf = true;
		root->n = 1;
		root->key[0] = newsFile;
		root->value[0] = pos;

		writeWord(root, BTree);
		delete root;

		rootInd = curInd;
		++curInd;

		return;
	}

	if (root->n == 2 * t - 1)
	{
		BNode* temp = root;
		root = new BNode;
		root->isLeaf = false;
		root->n = 0;
		root->child[0] = temp->index;

		split(root, 0, temp, curInd, BTree);

		root->index = curInd;

		write(root, BTree);

		rootInd = curInd;
		++curInd;
	}

	insertNonFullParaWord(root, newsFile, pos, curInd, BTree);
}