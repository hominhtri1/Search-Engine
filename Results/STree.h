#ifndef _STREE_H_
#define _STREE_H_

#include "Header.h"


struct SNode
{
	int nodePos;
	SNode* child[256];
	SNode* suffixLink;
	int start;
	int* end;
	int index;
};

struct childStruct
{
	int code; // ASCII code
	int pos;
};

struct diskNode
{
	int nodePos;
	int childNum;
	childStruct* child;
	int start;
	int end;
	int index;
};

struct activePoint
{
	SNode* node;
	int edge;
	int length;
};

SNode* newNode(int start, int* end, SNode* root);

int edgeLength(SNode* cur);

bool walkDown(SNode* cur, activePoint& point);

void extendSuffixTree(int pos, int* leafEnd, int& remainCount, activePoint& point, string text, SNode* root);

void setIndex(SNode* cur, int height, int length);

SNode* buildSuffixTree(string text);

void print(SNode* cur, string text);

void write(SNode* cur, ofstream& fout);

void suffixTreeToFile(SNode* root, ofstream& STreeFile, ofstream& metaFile);

diskNode* read(int pos, ifstream& fin);

void clear(SNode* &cur, bool& deleteLeaf);

diskNode* substringNode(string text, string query, ifstream& fin, ifstream& metaFile);

void fillBeginPos(diskNode* cur, ifstream& fin, bool* beginPos);

void substringBeginPos(string text, string query, ifstream& fin, bool* beginPos, ifstream& metaFile);

void wordsWithWildcardBeginPos(string text, string query, ifstream& fin, bool* wwwbeginPos, ifstream& metaFile);

bool isDelimiter(char ch);

bool isWordPosDelimiter(string text, int pos);

//void buildWordPos(string text, ofstream& wordPosFile, ofstream& metaFile);

int buildWordPos(string text, fstream& wordPosFile);

void prepareWordPos(int start, int end);

bool isWordAsterisk(string query, int qPos);

int posAfterOrEqualWord(int pos, ifstream& wordPosFile);

int wordAtPos(int wordPos, ifstream& wordPosFile);

bool exist(string text, string query, ifstream& STree, ifstream& wordPosFile, ifstream& metaFile);

int* positions(string text, string query, ifstream& STree, ifstream& wordPosFile, ifstream& metaFile, int& count);

void prepareSuffixTreeData(int newsFile);

bool existInNewsFileSTree(int newsFile, string query);

void existInAllNewsFileSTree(string query, bool* exist);

int* positionsInNewsFile(int newsFile, string query, int& count);

#endif