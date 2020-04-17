#ifndef _BTREE_H_
#define _BTREE_H_

#include "Header.h"

const int t = 15;

struct BNode
{
	int index;
	int n;
	int key[2 * t - 1];
	int value[2 * t - 1];
	int tempValue[2 * t - 1];
	bool isLeaf;
	int child[2 * t];
};

struct fixedStr
{
	char str[128];
};

const int strSize = 128;

bool operator<(const fixedStr & a, const fixedStr & b);

bool operator==(const fixedStr & a, const fixedStr & b);

bool operator>(const fixedStr & a, const fixedStr & b);

struct BNodeStr
{
	int index;
	int n;
	fixedStr key[2 * t - 1];
	int value[2 * t - 1];
	int tempValue[2 * t - 1];
	bool isLeaf;
	int child[2 * t];
};

struct tempNode
{
	int pos;
	int count;
};

struct valueNode
{
	int pos;
	int fileInd;
	int count;
	int curCount;
};

class SearchResult {
public:
	//No. of this file, e.g: fileInd = 98 is the file Group05News98.txt
	int fileInd;

	//frequency of the word in this file
	int count;

	//places of the word in the files
	int*positions;

	~SearchResult() {
		delete[] positions;
	}
};

class SearchResultArray {
public:

	//The number of files containing the word
	int filesCount;

	//Array of those files
	SearchResult*searchResultArray;

	~SearchResultArray() {
		delete[] searchResultArray;
	}
};

BNode* read(int ind, fstream& BTree);

BNodeStr* readStr(int ind, fstream& BTree);

void write(BNode* cur, fstream& BTree);

void writeStr(BNodeStr* cur, fstream& BTree);

int writeToTempFile(fstream& tempFile);

void updateTempFile(fstream& tempFile, int pos);

int writeToValueFile(fstream& valueFile, int newsFile, int count, int firstPos);

void updateValueFile(fstream& valueFile, int pos, int newsFile, int count, int wordPos);

void split(BNode* cur, int ind, BNode* indCur, int& curInd, fstream& BTree);

void splitStr(BNodeStr* cur, int ind, BNodeStr* indCur, int& curInd, fstream& BTree);

void insertNonFull(BNode* cur, int k, int& curInd, fstream& BTree, fstream& tempFile);

void insertNonFullStr(BNodeStr* cur, fixedStr k, int& curInd, fstream& BTree, fstream& tempFile);

void insert(int& rootInd, int k, int& curInd, fstream& BTree, fstream& tempFile);

void insertStr(int& rootInd, fixedStr k, int& curInd, fstream& BTree, fstream& tempFile);

void fillExist(fstream& valueFile, int pos, bool* exist);

void range(int curInd, fstream& BTree, fstream& valueFile, int start, int end, bool* exist);

void countRange(int curInd, fstream& BTree, int start, int end, int& count);

void updateRangePositions(int curInd, int newsFile, fstream& BTree, fstream& valueFile, int start, int end, int* &curPositions, int& curCount);

bool isNumber(string text, int curPos);

bool isDelimiter(string text, int curPos);

void generateRandFiles(int numFiles, int fileSize, int numRange);

void prepareBTreeData(int startFile, int endFile);

void prepareBTreeDataStr2(int startFile, int endFile);

void prepareBTreeDataStr(int& startFile, int& endFile, int index);

void rangeExist(int startNum, int endNum, bool* exist);

int numPos(int curInd, int k, fstream& BTree);

int strPos(int curInd, fixedStr k, fstream& BTree);

BNode* containingNode(int curInd, int k, fstream& BTree);

BNodeStr* containingNodeStr(int curInd, fixedStr k, fstream& BTree);

void updateValue(int rootInd, int k, int kPos, int curNewsFile, fstream& BTree, fstream& valueFile, fstream& tempFile);

void updateValueStr(int rootInd, fixedStr k, int kPos, int curNewsFile, fstream& BTree, fstream& valueFile, fstream& tempFile);

int* positionsInFile(fstream& valueFile, int pos, int newsFile, int& count);

int countInFile(fstream& valueFile, int pos, int newsFile);

void existInNewsFileBTree(fixedStr k, bool* exist);

int* wordPositions(int newsFile, fixedStr k, int& count);

void printPositions(int newsFile, fixedStr k);

int* rangePositions(int newsFile, int start, int end, int& count);

//-------------------------------------------------------------------------------------------------

//This is for Ngoc Yen, look for SearchResultArray class and the below test function to know more.
SearchResultArray* searchFor(fixedStr k);

//Test printing out the results 
void printPositions(fixedStr k);

void existInNewsFileTitleBTree(fixedStr k, bool* exist);

//-------------------------------------------------------------------------------------------------

void writePara(BNode* cur, fstream& BTree);

void writeWord(BNode* cur, fstream& BTree);

void insertNonFullParaWord(BNode* cur, int newsFile, int pos, int& curInd, fstream& BTree);

void insertPara(int& rootInd, int newsFile, int pos, int& curInd, fstream& BTree);

void insertWord(int& rootInd, int newsFile, int pos, int& curInd, fstream& BTree);

#endif