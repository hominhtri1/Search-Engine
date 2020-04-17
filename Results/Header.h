#ifndef _HEADER_H_
#define _HEADER_H_

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <algorithm>
#include <vector>
#include <chrono>
#include <filesystem>
#include <conio.h>

namespace fs = std::experimental::filesystem;

using namespace std;

const int newsFileCount = 2000;

const string rootPath = "";

const string rootPathSynonym = "raw\\";
const string synonymFileName = "synonym.txt";

const string rootNewsFolder = "news\\";

const string rootPathStopWord = "raw\\";
const string stopWordFileName = "stopword.txt";

struct fileScorePair
{
	int file;
	double score;
};

struct paraScorePair
{
	int para;
	double score;
};

struct posNumPair
{
	int pos;
	int wordNum;
};

bool operator<(const fileScorePair & a, const fileScorePair & b);

bool comp1(const paraScorePair & a, const paraScorePair & b);

bool comp2(const paraScorePair & a, const paraScorePair & b);

//-------------------------------------------

int buildParagraphPos(string text, fstream& paraPosFile);

void prepareParagraphPos(int start, int end);

int firstParagraphEnding(fstream& paraPosFile, int paraFilePos, int newsFile);

bool isScoreDelimiter(string text, int pos);

double scoreOfFile(int newsFile, fstream& paraPosFile, string query, int paraFilePos);

int topNewsFile(string query, int* topFile);

int phraseLength(string phrase);

int printPos(int newsFile, fstream& paraPosFile, string query, int paraFilePos, int* startPos, posNumPair** hili);

int highlightPositions(int newsFile, string query, int* startPos, posNumPair** hili);

//-------------------------------------------

bool isHighlightDelimeter(string text, int curPos);

void delete_non_ascii(string& para);

#endif