#ifndef _BTREESTOPWORD_H_
#define _BTREESTOPWORD_H_

#include "Header.h"
#include "BTree.h"

struct BNodeStopWord
{
	int index;
	int n;
	fixedStr key[2 * t - 1];
	bool isLeaf;
	int child[2 * t];
};

BNodeStopWord* readStrStopWord(int ind, fstream& BTree);

void writeStrStopWord(BNodeStopWord* cur, fstream& BTree);

void splitStrStopWord(BNodeStopWord* cur, int ind, BNodeStopWord* indCur, int& curInd, fstream& BTree);

void insertNonFullStrStopWord(BNodeStopWord* cur, fixedStr k, int& curInd, fstream& BTree);

void insertStrStopWord(int& rootInd, fixedStr k, int& curInd, fstream& BTree);

//bool isDelimiter(string text, int curPos);

void prepareBTreeStopWord();

bool strPosStopWord(int curInd, fixedStr k, fstream& BTree);

//Return a vector!
bool isStopWord(fixedStr k);

//Test printing out the results 
void testStopWord(fixedStr k);

#endif