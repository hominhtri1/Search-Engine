#ifndef _RTREE_H_
#define _RTREE_H_

#include "BTree.h"


const string emptyStr = "                                                                                                                                                                 ";

//--------------------------

void setDisplay();

void gotoxy(int x, int y);

void setColor(int color);

//--------------------------

struct RNode
{
	fixedStr path;
	bool isLeaf = true;
	int child[256];
	int fileName = -1;

	RNode()
	{
		for (int i = 0; i < 256; ++i) child[i] = -1;
	}
};

bool find(RNode* root, string k, string* &top5, fstream& RTree);

void traverse(string result, int& count, string* &top5, RNode* cur, fstream& RTree);

void print(RNode* root, string str, string* &top5, bool goFind, fstream& RTree);

void writeRTree(RNode* cur, int curInd, fstream& RTree);

void readRTree(RNode* &cur, int fileName, fstream& RTree);

void insertTree(RNode* &root, string k, int& fileIndex, fstream& RTree);

//--------------------------

void intro();

void welcome();

void perform(RNode* &root);

void show(string query);

void press(int* topFile, int i, string query);

//--------------------------

void query();


#endif // !_RTREE_H_
