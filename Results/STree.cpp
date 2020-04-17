#include "STree.h"
#include "BTree.h"
#include "BTreeNewsPath.h"

SNode* newNode(int start, int* end, SNode* root)
{
	SNode* cur = new SNode;

	for (int i = 0; i < 256; ++i)
		cur->child[i] = NULL;

	cur->suffixLink = root;
	cur->start = start;
	cur->end = end;
	cur->index = -1;

	return cur;
}

int edgeLength(SNode* cur)
{
	return *cur->end - cur->start + 1;
}

bool walkDown(SNode* cur, activePoint& point)
{
	int curLength = edgeLength(cur);

	if (curLength <= point.length)
	{
		point.node = cur;
		point.edge += curLength;
		point.length -= curLength;

		return true;
	}
	else
		return false;
}

void extendSuffixTree(int pos, int* leafEnd, int& remainCount, activePoint& point, string text, SNode* root)
{
	*leafEnd = pos;

	++remainCount;

	SNode* lastNewNode = NULL;

	while (remainCount > 0)
	{
		if (0 == point.length)
			point.edge = pos;

		if (NULL == point.node->child[text[point.edge]])
		{
			point.node->child[text[point.edge]] = newNode(pos, leafEnd, root);

			if (lastNewNode != NULL)
			{
				lastNewNode->suffixLink = point.node;
				lastNewNode = NULL;
			}
		}
		else
		{
			SNode* next = point.node->child[text[point.edge]];

			if (walkDown(next, point))
				continue;

			if (text[next->start + point.length] == text[pos])
			{
				if (lastNewNode != NULL)
				{
					lastNewNode->suffixLink = point.node;
					lastNewNode = NULL;
				}

				++point.length;

				break;
			}

			int* splitEnd = new int;
			*splitEnd = next->start + point.length - 1;

			SNode* split = newNode(next->start, splitEnd, root);
			point.node->child[text[point.edge]] = split;

			split->child[text[pos]] = newNode(pos, leafEnd, root);
			next->start += point.length;
			split->child[text[next->start]] = next;

			if (lastNewNode != NULL)
				lastNewNode->suffixLink = split;

			lastNewNode = split;
		}

		--remainCount;

		if (point.node == root && point.length > 0)
		{
			--point.length;
			point.edge = pos - remainCount + 1;
		}
		else if (point.node != root)
			point.node = point.node->suffixLink;
	}
}

void setIndex(SNode* cur, int curHeight, int length)
{
	if (NULL == cur)
		return;

	bool isLeaf = true;

	for (int i = 0; i < 256; ++i)
	{
		if (cur->child[i] != NULL)
		{
			isLeaf = false;

			setIndex(cur->child[i], curHeight + edgeLength(cur->child[i]), length);
		}
	}

	if (isLeaf)
		cur->index = length - curHeight;
}

SNode* buildSuffixTree(string text)
{
	int* rootEnd = new int;
	*rootEnd = -1;

	SNode* root = new SNode;

	for (int i = 0; i < 256; ++i)
		root->child[i] = NULL;

	root->suffixLink = NULL;
	root->start = -1;
	root->end = rootEnd;
	root->index = -1;

	activePoint point;
	point.node = root;
	point.edge = -1;
	point.length = 0;

	int length = text.length();
	int* leafEnd = new int;
	int remainCount = 0;

	for (int i = 0; i < length; ++i)
		extendSuffixTree(i, leafEnd, remainCount, point, text, root);

	setIndex(root, 0, length);

	return root;
}

void print(SNode* cur, string text)
{
	if (NULL == cur)
		return;

	if (-1 == cur->index)
		for (int i = 0; i < 256; ++i)
			print(cur->child[i], text);

	int length = text.length();

	//	for (int i = cur->start; i <= *(cur->end); ++i)
	//	{
	//		if (i >= 0 && i < length)
	//			cout << text[i];
	//	}

	cout << cur->start << " " << *(cur->end);

	cout << endl;
}

void write(SNode* cur, ofstream& fout)
{
	if (NULL == cur)
		return;

	if (-1 == cur->index)
		for (int i = 0; i < 256; ++i)
			write(cur->child[i], fout);

	cur->nodePos = fout.tellp();
	fout << cur->nodePos << endl;

	if (-1 == cur->index)
	{
		int childNum = 0;

		for (int i = 0; i < 256; ++i)
		{
			if (cur->child[i] != NULL)
				++childNum;
		}

		fout << childNum << endl;

		for (int i = 0; i < 256; ++i)
		{
			if (cur->child[i] != NULL)
				fout << i << " " << cur->child[i]->nodePos << endl;
		}
	}
	else
		fout << 0 << endl;

	fout << cur->start << endl;
	fout << *(cur->end) << endl;
	fout << cur->index << endl;
}

void suffixTreeToFile(SNode* root, ofstream& STreeFile, ofstream& metaFile)
{
	write(root, STreeFile);

	metaFile << root->nodePos << endl;
}

diskNode* read(int pos, ifstream& fin)
{
	fin.seekg(pos, fin.beg);

	diskNode* cur = new diskNode;

	fin >> cur->nodePos;
	fin >> cur->childNum;
	cur->child = new childStruct[cur->childNum];

	for (int i = 0; i < cur->childNum; ++i)
		fin >> cur->child[i].code >> cur->child[i].pos;

	fin >> cur->start;
	fin >> cur->end;
	fin >> cur->index;

	return cur;
}

void clear(SNode* &cur, bool& deleteLeaf)
{
	if (NULL == cur)
		return;

	if (-1 == cur->index)
		for (int i = 0; i < 256; ++i)
			clear(cur->child[i], deleteLeaf);

	if (-1 == cur->index)
		delete cur->end;
	else
	{
		if (deleteLeaf)
			delete cur->end;

		deleteLeaf = false;
	}

	delete cur;

	cur = NULL;
}

diskNode* substringNode(string text, string query, ifstream& fin, ifstream& metaFile)
{
	int rootPos;
	metaFile.seekg(0, metaFile.beg);
	metaFile >> rootPos;
	diskNode* cur = read(rootPos, fin);

	int length = query.length();
	int curPos = 0;

	while (true)
	{
		int ind = query[curPos];

		bool found = false;
		int i = 0;

		while (!found && i < cur->childNum)
		{
			if (cur->child[i].code == ind)
				found = true;
			else
				++i;
		}

		if (!found)
		{
			delete[] cur->child;
			delete cur;

			return NULL;
		}

		diskNode* next = read(cur->child[i].pos, fin);

		delete[] cur->child;
		delete cur;

		i = next->start;

		while (i <= next->end && curPos < length && text[i] == query[curPos])
		{
			++i;
			++curPos;
		}

		if (curPos == length)
			return next;
		else if (i > next->end)
			cur = next;
		else
		{
			delete[] next->child;
			delete next;

			return NULL;
		}
	}
}

void fillBeginPos(diskNode* cur, ifstream& fin, bool* beginPos)
{
	if (cur->index != -1)
		beginPos[cur->index] = true;
	else
	{
		int i = 0;

		while (i < cur->childNum)
		{
			diskNode* next = read(cur->child[i].pos, fin);

			fillBeginPos(next, fin, beginPos);

			++i;
		}
	}

	delete[] cur->child;
	delete[] cur;
}

void substringBeginPos(string text, string query, ifstream& fin, bool* beginPos, ifstream& metaFile)
{
	diskNode* cur = substringNode(text, query, fin, metaFile);

	for (int i = 0; i < text.length(); ++i)
		beginPos[i] = false;

	if (cur != NULL)
		fillBeginPos(cur, fin, beginPos);
}

void wordsWithWildcardBeginPos(string text, string query, ifstream& fin, bool* wwwBeginPos, ifstream& metaFile)
{
	int tLength = text.length();
	int qLength = query.length();

	int* beginPosCount = new int[tLength];
	for (int i = 0; i < tLength; ++i)
		beginPosCount[i] = 0;

	bool* beginPos = new bool[tLength];

	int tokenCount = 0;

	int qPos = 0;

	while (qPos < qLength)
	{
		while (qPos < qLength && '*' == query[qPos])
			++qPos;

		if (qPos == qLength)
			break;

		++tokenCount;

		int tempPos = qPos;

		while (tempPos < qLength - 1 && query[tempPos + 1] != '*')
			++tempPos;

		string token = query.substr(qPos, tempPos - qPos + 1);

		substringBeginPos(text, token, fin, beginPos, metaFile);

		for (int i = 0; i < tLength; ++i)
		{
			if (beginPos[i] && i - qPos >= 0)
				++beginPosCount[i - qPos];
		}

		qPos = tempPos + 1;
	}

	for (int i = 0; i < tLength; ++i)
	{
		if (beginPosCount[i] == tokenCount)
			wwwBeginPos[i] = true;
		else
			wwwBeginPos[i] = false;
	}

	delete[] beginPosCount;
	delete[] beginPos;
}

bool isDelimiter(char ch)
{
	return (' ' == ch || 10 == ch);
}

bool isWordPosDelimiter(string text, int pos)
{
	return (' ' == text[pos] || 10 == text[pos]);
}

int buildWordPos(string text, fstream& wordPosFile)
{
	int tLength = text.length();

	int wordCount = 0;

	int curPos = 0;
	int tempPos;

	while (curPos < tLength)
	{
		while (curPos < tLength && isWordPosDelimiter(text, curPos))
			++curPos;

		if (curPos == tLength)
			break;

		++wordCount;

		tempPos = curPos;

		while (tempPos < tLength - 1 && !isWordPosDelimiter(text, tempPos + 1))
			++tempPos;

		curPos = tempPos + 1;
	}

	int* wordPos = new int[wordCount + 2];
	wordPos[0] = wordCount;

	int i = 1;

	curPos = 0;

	while (curPos < tLength)
	{
		while (curPos < tLength && isWordPosDelimiter(text, curPos))
			++curPos;

		if (curPos == tLength)
			break;

		wordPos[i] = curPos;

		++i;

		tempPos = curPos;

		while (tempPos < tLength - 1 && !isWordPosDelimiter(text, tempPos + 1))
			++tempPos;

		curPos = tempPos + 1;
	}

	wordPos[wordCount + 1] = text.length();

	int temp;

	wordPosFile.seekp(0, wordPosFile.end);
	int endPos = wordPosFile.tellp();
	wordPosFile.write((char*)wordPos, (wordCount + 2) * (sizeof temp));

	delete[] wordPos;

	return endPos;
}

void prepareWordPos(int start, int end)
{
	int rootInd, curInd;

	string metaFilePath = rootPath + "metaWord.txt";

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

	fstream BTree, wordPosFile;

	string BTreePath = rootPath + string("BTreeWord.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string wordPath = rootPath + "word.txt";

	ofstream fout;
	fout.open(wordPath, ios::app);

	fout.close();

	wordPosFile.open(wordPath, fstream::binary | fstream::out | fstream::in);

	for (int i = start; i <= end; ++i)
	{
		string path = getNewsPaths(i);

		ifstream fin;
		fin.open(path);

		stringstream buffer;
		buffer << fin.rdbuf();

		fin.close();

		string text = buffer.str();

		int curPos = buildWordPos(text, wordPosFile);

		insertWord(rootInd, i, curPos, curInd, BTree);
	}

	BTree.close();
	wordPosFile.close();

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

bool isWordAsterisk(string query, int qPos)
{
	int qLength = query.length();

	return ('*' == query[qPos] && (0 == qPos || isDelimiter(query[qPos - 1])) &&
		(qPos == qLength - 1 || isDelimiter(query[qPos + 1])));
}

int posAfterOrEqualWord(int pos, ifstream& wordPosFile)
{
	wordPosFile.seekg(0, wordPosFile.beg);

	int curCount = 0;

	int temp;
	wordPosFile >> temp;

	while (temp != -1 && temp <= pos)
	{
		++curCount;
		wordPosFile >> temp;
	}

	return (curCount - 1);
}

int wordAtPos(int wordPos, ifstream& wordPosFile)
{
	wordPosFile.seekg(0, wordPosFile.beg);

	int curCount = 0;

	int temp;
	wordPosFile >> temp;

	while (curCount < wordPos)
	{
		++curCount;
		wordPosFile >> temp;
	}

	return temp;
}

bool exist(string text, string query, ifstream& STree, ifstream& wordPosFile, ifstream& metaFile)
{
	int dummy, numWord;
	metaFile.seekg(0, metaFile.beg);
	metaFile >> dummy >> numWord;
	int* beginWordPosCount = new int[numWord];
	for (int i = 0; i < numWord; ++i)
		beginWordPosCount[i] = 0;

	int tLength = text.length();
	int qLength = query.length();

	bool* wwwBeginPos = new bool[tLength];

	int tokenCount = 0;
	int curWordCount = 0;

	int qPos = 0;

	while (qPos < qLength)
	{
		if (isWordAsterisk(query, qPos))
		{
			++curWordCount;
			++qPos;
		}

		if (qPos == qLength)
			break;

		++tokenCount;

		int tempPos = qPos;

		int tempWordCount = 0;

		if (0 == qPos && !isDelimiter(query[0]))
			++tempWordCount;

		while (tempPos < qLength - 1 && !isWordAsterisk(query, tempPos + 1))
		{
			if (isDelimiter(query[tempPos]) && !isDelimiter(query[tempPos + 1]))
				++tempWordCount;

			++tempPos;
		}

		string words = query.substr(qPos, tempPos - qPos + 1);

		wordsWithWildcardBeginPos(text, words, STree, wwwBeginPos, metaFile);

		for (int i = 0; i < tLength; ++i)
		{
			if (wwwBeginPos[i])
			{
				int wordPos = posAfterOrEqualWord(i, wordPosFile);

				if (0 == qPos && !isDelimiter(query[0]))
					++beginWordPosCount[wordPos];
				else if (wordPos - curWordCount + 1 >= 0)
					++beginWordPosCount[wordPos - curWordCount + 1];
			}
		}

		curWordCount += tempWordCount;

		qPos = tempPos + 1;
	}

	int i = 0;

	while (i < numWord && beginWordPosCount[i] != tokenCount)
		++i;

	delete[] beginWordPosCount;
	delete[] wwwBeginPos;

	return (i < numWord);
}

int* positions(string text, string query, ifstream& STree, ifstream& wordPosFile, ifstream& metaFile, int& count)
{
	int dummy, numWord;
	metaFile.seekg(0, metaFile.beg);
	metaFile >> dummy >> numWord;
	int* beginWordPosCount = new int[numWord];
	for (int i = 0; i < numWord; ++i)
		beginWordPosCount[i] = 0;

	int tLength = text.length();
	int qLength = query.length();

	bool* wwwBeginPos = new bool[tLength];

	int tokenCount = 0;
	int curWordCount = 0;

	int qPos = 0;

	while (qPos < qLength)
	{
		if (isWordAsterisk(query, qPos))
		{
			++curWordCount;
			++qPos;
		}

		if (qPos == qLength)
			break;

		++tokenCount;

		int tempPos = qPos;

		int tempWordCount = 0;

		if (0 == qPos && !isDelimiter(query[0]))
			++tempWordCount;

		while (tempPos < qLength - 1 && !isWordAsterisk(query, tempPos + 1))
		{
			if (isDelimiter(query[tempPos]) && !isDelimiter(query[tempPos + 1]))
				++tempWordCount;

			++tempPos;
		}

		string words = query.substr(qPos, tempPos - qPos + 1);

		wordsWithWildcardBeginPos(text, words, STree, wwwBeginPos, metaFile);

		for (int i = 0; i < tLength; ++i)
		{
			if (wwwBeginPos[i])
			{
				int wordPos = posAfterOrEqualWord(i, wordPosFile);

				if (0 == qPos && !isDelimiter(query[0]))
					++beginWordPosCount[wordPos];
				else if (wordPos - curWordCount + 1 >= 0)
					++beginWordPosCount[wordPos - curWordCount + 1];
			}
		}

		curWordCount += tempWordCount;

		qPos = tempPos + 1;
	}

	count = 0;

	for (int i = 0; i < numWord; ++i)
	{
		if (beginWordPosCount[i] == tokenCount)
			++count;
	}

	if (0 == count)
	{
		delete[] beginWordPosCount;
		delete[] wwwBeginPos;

		return NULL;
	}
	else
	{
		int* pos = new int[count];

		int posInd = 0;

		for (int i = 0; i < numWord; ++i)
		{
			if (beginWordPosCount[i] == tokenCount)
			{
				pos[posInd] = wordAtPos(i, wordPosFile);
				++posInd;
			}
		}

		delete[] beginWordPosCount;
		delete[] wwwBeginPos;

		return pos;
	}
}

void prepareSuffixTreeData(int newsFile)
{
	// Put the news files into subdirectory rootPath\\News files
	string path = rootPath + "\\News files\\Group05News" + to_string(newsFile) + ".txt";

	ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		cout << "Can't open news file to read prepareSuffixTreeData" << endl;
		return;
	}

	// Put all of the file "fin" into a string "text" in one go
	stringstream buffer;
	buffer << fin.rdbuf();

	fin.close();

	string text = buffer.str() + '`';

	// Creating the meta file
	string metaFilePath = rootPath + "News" + to_string(newsFile) + "meta.txt";

	ofstream metaFile;
	metaFile.open(metaFilePath);
	if (!metaFile.is_open())
	{
		cout << "Can't open meta file to write" << endl;
		return;
	}

	// Creating the suffix tree file
	SNode* root = buildSuffixTree(text);

	string STreePath = rootPath + "News" + to_string(newsFile) + "STree.txt";

	ofstream STreeFile;
	STreeFile.open(STreePath);
	if (!STreeFile.is_open())
	{
		cout << "Can't open suffix tree file to write" << endl;
		return;
	}

	suffixTreeToFile(root, STreeFile, metaFile);

	STreeFile.close();

	bool deleteLeaf = true;

	clear(root, deleteLeaf);
	/*
	// Creating the wordPos file
	string wordPosPath = rootPath + "News" + to_string(newsFile) + "WordPos.txt";

	ofstream wordPosFile;
	wordPosFile.open(wordPosPath);
	if (!wordPosFile.is_open())
	{
	cout << "Can't open wordPos file to write" << endl;
	return;
	}

	//	buildWordPos(text, wordPosFile, metaFile);

	wordPosFile.close();
	*/
	metaFile.close();
}

bool existInNewsFileSTree(int newsFile, string query)
{
	string path = rootPath + "\\News files\\Group05News" + to_string(newsFile) + ".txt";

	ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		//		cout << "Can't open news file to read" << endl;
		return false;
	}

	stringstream buffer;
	buffer << fin.rdbuf();

	fin.close();

	string text = buffer.str() + '`';

	string metaFilePath = rootPath + "Suffix files\\News" + to_string(newsFile) + "meta.txt";

	ifstream metaFile;

	metaFile.open(metaFilePath);
	if (!metaFile.is_open())
	{
		//		cout << "Can't open meta file to read" << endl;
		return false;
	}

	string STreePath = rootPath + "Suffix files\\News" + to_string(newsFile) + "STree.txt";

	ifstream STreeFile;

	STreeFile.open(STreePath);
	if (!STreeFile.is_open())
	{
		//		cout << "Can't open suffix tree file to read" << endl;
		return false;
	}
	/*
	string wordPosPath = rootPath + "Suffix files\\News" + to_string(newsFile) + "WordPos.txt";

	ifstream wordPosFile;

	wordPosFile.open(wordPosPath);
	if (!wordPosFile.is_open())
	{
	//		cout << "Can't open wordPos file to read" << endl;
	return false;
	}
	*/
	//	bool exists = exist(text, query, STreeFile, wordPosFile, metaFile);

	metaFile.close();
	STreeFile.close();
	//	wordPosFile.close();

	return NULL;
	//	return exists;
}

void existInAllNewsFileSTree(string query, bool* exist)
{
	int rootIndWord;

	string metaFilePathWord = rootPath + "metaWord.txt";

	ifstream metaReadWord;
	metaReadWord.open(metaFilePathWord);
	if (!metaReadWord.is_open())
	{
		cout << "Can't open meta file to read" << endl;
		return;
	}

	metaReadWord >> rootIndWord;

	metaReadWord.close();

	string BTreeWordPath = rootPath + "BTreeWord.txt";
	string wordPath = rootPath + "word.txt";

	fstream BTreeWord, wordPosFile;

	BTreeWord.open(BTreeWordPath, fstream::binary | fstream::out | fstream::in);
	wordPosFile.open(wordPath, fstream::binary | fstream::out | fstream::in);

	int newsFile = 83;

	int pos = numPos(rootIndWord, newsFile, BTreeWord);

	BTreeWord.close();
	wordPosFile.close();
}

int* positionsInNewsFile(int newsFile, string query, int& count)
{
	string path = rootPath + "\\News files\\Group05News" + to_string(newsFile) + ".txt";

	ifstream fin;
	fin.open(path);
	if (!fin.is_open())
	{
		cout << "Can't open news file to read positionsInNewsFile" << endl;
		return false;
	}

	stringstream buffer;
	buffer << fin.rdbuf();

	fin.close();

	string text = buffer.str() + '`';

	string metaFilePath = rootPath + "Suffix files\\News" + to_string(newsFile) + "meta.txt";

	ifstream metaFile;

	metaFile.open(metaFilePath);
	if (!metaFile.is_open())
	{
		cout << "Can't open meta file to read" << endl;
		return false;
	}

	string STreePath = rootPath + "Suffix files\\News" + to_string(newsFile) + "STree.txt";

	ifstream STreeFile;

	STreeFile.open(STreePath);
	if (!STreeFile.is_open())
	{
		cout << "Can't open suffix tree file to read" << endl;
		return false;
	}

	string wordPosPath = rootPath + "Suffix files\\News" + to_string(newsFile) + "WordPos.txt";

	ifstream wordPosFile;

	wordPosFile.open(wordPosPath);
	if (!wordPosFile.is_open())
	{
		cout << "Can't open wordPos file to read" << endl;
		return false;
	}

	int* pos = positions(text, query, STreeFile, wordPosFile, metaFile, count);

	metaFile.close();
	STreeFile.close();
	wordPosFile.close();

	return pos;
}