#ifndef _BTREENEWSPATH_H_
#define _BTREENEWSPATH_H_

#include "Header.h"

const int tNewsPath = 22;

struct fixedStrNewsPath
{
	char str[256];
};

const int strNewsPathSize = 256;

struct BNodeStrNewPath
{
	int index;
	int n;
	fixedStrNewsPath value[2 * tNewsPath - 1];
	int key[2 * tNewsPath - 1];
	bool isLeaf;
	int child[2 * tNewsPath];
};

bool operator<(const fixedStrNewsPath & a, const fixedStrNewsPath & b);

bool operator==(const fixedStrNewsPath & a, const fixedStrNewsPath & b);

bool operator>(const fixedStrNewsPath & a, const fixedStrNewsPath & b);

BNodeStrNewPath* readStrNewsPath(int ind, fstream& BTreeNewsPath);

void writeStrNewsPath(BNodeStrNewPath* cur, fstream& BTreeNewsPath);

void splitStrNewsPath(BNodeStrNewPath* cur, int ind, BNodeStrNewPath* indCur, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath);

void insertNonFullStrNewsPath(BNodeStrNewPath* cur, int k, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath);

void insertStrNewsPath(int& rootIndNewsPath, int k, int& curIndNewsPath, fstream& BTreeNewsPath, fixedStrNewsPath newsPath);

string strPosNewsPath(int curIndNewsPath, int k, fstream& BTreeNewsPath);

//Return a vector!
string getNewsPaths(int k);

//Test printing out the results 
void testNewsPath(int k);

#endif