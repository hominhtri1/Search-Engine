#ifndef _SYNONYM_H_
#define _SYNONYM_H_

#include "Header.h"
#include "BTree.h"

class SynonymsArray {
public:
	//synonynm count
	int wordCount;

	//Array of those files
	vector<fixedStr> synonymsArray;

};

void writeStrSyn(BNodeStr* cur, fstream& BTree);

void splitStrSyn(BNodeStr* cur, int ind, BNodeStr* indCur, int& curInd, fstream& BTree, int tellg);

void insertNonFullStrSyn(BNodeStr* cur, fixedStr k, int& curInd, fstream& BTree, int tellg);

void insertStrSyn(int& rootInd, fixedStr k, int& curInd, fstream& BTree, int tellg);

void prepareBTreeSynonym();

bool isDelimiterSyn(string text, int curPos);

//Return a vector!
SynonymsArray* getSynonyms(fixedStr k);

//Test printing out the results 
void testSynonym(fixedStr k);

void existInNewsFileBTreeSyn(fixedStr k, bool* exist);

int* wordPositionsSyn(int newsFile, fixedStr k, int& curCount);

#endif