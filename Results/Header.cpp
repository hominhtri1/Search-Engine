#include "Header.h"
#include "BTree.h"
#include "STree.h"
#include "Polish.h"
#include "BTreeNewsPath.h"
#include "BTreeStopWord.h"
#include "Synonym.h"


bool operator<(const fileScorePair& a, const fileScorePair& b)
{
	return (a.score > b.score);
}

bool comp1(const paraScorePair& a, const paraScorePair& b)
{
	return (a.score > b.score);
}

bool comp2(const paraScorePair& a, const paraScorePair& b)
{
	return (a.para < b.para);
}

int buildParagraphPos(string text, fstream& paraPosFile)
{
	int tLength = text.length();

	int paraCount = 0;

	int curPos = 0;
	int tempPos;

	while (curPos < tLength)
	{
		while (curPos < tLength && 10 == text[curPos])
			++curPos;

		if (curPos == tLength)
			break;

		++paraCount;

		tempPos = curPos;

		while (tempPos < tLength - 1 && text[tempPos + 1] != 10)
			++tempPos;

		curPos = tempPos + 1;
	}

	int* paraPos = new int[paraCount + 2];
	paraPos[0] = paraCount;

	int i = 1;

	curPos = 0;

	while (curPos < tLength)
	{
		while (curPos < tLength && 10 == text[curPos])
			++curPos;

		if (curPos == tLength)
			break;

		paraPos[i] = curPos;

		++i;

		tempPos = curPos;

		while (tempPos < tLength - 1 && text[tempPos + 1] != 10)
			++tempPos;

		curPos = tempPos + 1;
	}

	paraPos[paraCount + 1] = text.length();

	int temp;

	paraPosFile.seekp(0, paraPosFile.end);
	int endPos = paraPosFile.tellp();
	paraPosFile.write((char*)paraPos, (paraCount + 2) * (sizeof temp));

	delete[] paraPos;

	return endPos;
}

void prepareParagraphPos(int start, int end)
{
	int rootInd, curInd;

	string metaFilePath = rootPath + "metaPara.txt";

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

	fstream BTree, paraPosFile;

	string BTreePath = rootPath + string("BTreeParagraph.txt");

	// Check to see if B-tree file is there
	// If it is, read it into BTree. If it isn't, do nothing
	ifstream tempFin;
	tempFin.open(BTreePath);
	if (tempFin.is_open())
	{
		tempFin.close();

		BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	}

	string paraPath = rootPath + "para.txt";

	ofstream fout;
	fout.open(paraPath, ios::app);

	fout.close();

	paraPosFile.open(paraPath, fstream::binary | fstream::out | fstream::in);

	for (int i = start; i <= end; ++i)
	{
		string path = getNewsPaths(i);

		ifstream fin;
		fin.open(path);

		stringstream buffer;
		buffer << fin.rdbuf();

		fin.close();

		string text = buffer.str();

		int curPos = buildParagraphPos(text, paraPosFile);

		insertPara(rootInd, i, curPos, curInd, BTree);
	}

	BTree.close();
	paraPosFile.close();

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

int firstParagraphEnding(fstream& paraPosFile, int paraFilePos, int newsFile)
{
	int paraNum = 0;

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)&paraNum, sizeof paraNum);
	paraPosFile.clear();

	// paraPos is 1-index
	int* paraPos = new int[paraNum + 2];

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)paraPos, (paraNum + 2) * (sizeof paraNum));
	paraPosFile.clear();

	int output = paraPos[2] - 1;

	delete[] paraPos;

	return output;
}

bool isScoreDelimiter(string text, int pos)
{
	return ('(' == text[pos] || ')' == text[pos] || ' ' == text[pos] || ':' == text[pos]);
}

double scoreOfFile(int newsFile, fstream& paraPosFile, string query, int paraFilePos)
{
	int paraNum = 0;

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)&paraNum, sizeof paraNum);
	paraPosFile.clear();

	// paraPos is 1-index
	int* paraPos = new int[paraNum + 2];

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)paraPos, (paraNum + 2) * (sizeof paraNum));
	paraPosFile.clear();

	//paraCount is 1-index
	int* paraCount = new int[paraNum + 1];
	for (int i = 0; i < paraNum + 1; ++i)
		paraCount[i] = 0;

	int length = query.length();

	int curPos = 0;
	int tempPos;

	string word;

	fixedStr k;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;

			word = query.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, word.c_str());

			if (word[0] == '+')
				word.erase(0, 1);

			if (word != "AND" && word != "OR" && (!isStopWord(k)))
			{
				if (word[0] == '~')
				{
					word.erase(0, 1);

					fixedStr k2;

					strcpy_s(k2.str, word.c_str());

					tokenPos = wordPositionsSyn(newsFile, k2, count);
				}
				else
				{
					bool ok = true;
					size_t found_1 = word.find('.'), found_2 = found_1 + 1;

					if (found_1 >= 0 && found_1 < word.length() - 1 && word[found_2] == '.')
					{
						for (int i = 0; i < found_1; i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						for (int i = found_2 + 1; i < word.length(); i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						if (ok == true)
						{
							string num1S = word.substr(0, found_1);
							string num2S = word.substr(found_2 + 1, word.length() - found_2 - 1);

							int num1 = stoi(num1S);
							int num2 = stoi(num2S);

							tokenPos = rangePositions(newsFile, num1, num2, count);
						}
						else
							tokenPos = wordPositions(newsFile, k, count);
					}
					else
						tokenPos = wordPositions(newsFile, k, count);
				}
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				++paraCount[paraInd];
				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	double tempScore = 0;

	// Can be adjusted
	for (int i = 0; i <= paraNum; ++i)
		tempScore += pow(paraCount[i] / ((paraPos[i + 1] - paraPos[i]) / 5.0), 2);

	tempScore /= paraPos[paraNum + 1];

	delete[] paraPos;
	delete[] paraCount;

	return tempScore;
}

/*double scoreOfFile(int newsFile, fstream& paraPosFile, string query, int paraFilePos)
{
	int paraNum = 0;

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)&paraNum, sizeof paraNum);
	paraPosFile.clear();

	// paraPos is 1-index
	int* paraPos = new int[paraNum + 2];

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)paraPos, (paraNum + 2) * (sizeof paraNum));
	paraPosFile.clear();

	//paraCount is 1-index
	int* paraCount = new int[paraNum + 1];
	for (int i = 0; i < paraNum + 1; ++i)
		paraCount[i] = 0;

	int length = query.length();

	int curPos = 0;
	int tempPos;

	string word;

	fixedStr k;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;


			word = query.substr(curPos, tempPos - curPos + 1);

			if (word != "AND" && word != "OR")
			{
				strcpy_s(k.str, word.c_str());

				tokenPos = wordPositions(newsFile, k, count);
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				++paraCount[paraInd];
				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	double tempScore = 0;

	// Can be adjusted
	for (int i = 0; i <= paraNum; ++i)
		tempScore += pow(paraCount[i] / ((paraPos[i + 1] - paraPos[i]) / 5.0), 2);

	tempScore /= paraPos[paraNum + 1];

	delete[] paraPos;
	delete[] paraCount;

	return tempScore;
}*/

int topNewsFile(string query, int* topFile)
{
	int rootInd;

	string metaFilePath = rootPath + "metaPara.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read" << endl;
		return 0;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeParagraph.txt";
	string paraPath = rootPath + "para.txt";

	fstream BTree, paraPosFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	paraPosFile.open(paraPath, fstream::binary | fstream::out | fstream::in);

	init(query);

	bool* exist = new bool[newsFileCount];

	int count = resolveInput(query, exist);

	vector<fileScorePair> fileList(count);

	int i = 0, j = 0;

	while (j < newsFileCount)
	{
		if (exist[j])
		{
			fileList[i].file = j;

			int pos = numPos(rootInd, fileList[i].file, BTree);

			if (-1 == pos)
				fileList[i].score = 0;
			else
				fileList[i].score = scoreOfFile(fileList[i].file, paraPosFile, query, pos);

			++i;
		}

		++j;
	}

	BTree.close();
	paraPosFile.close();

	delete[] exist;

	sort(fileList.begin(), fileList.end());

	if (count >= 20)
	{
		for (int i = 0; i < 20; ++i)
			topFile[i] = fileList[i].file;

		return 20;
	}
	else
	{
		for (int i = 0; i < count; ++i)
			topFile[i] = fileList[i].file;

		return count;
	}
}

int phraseLength(string phrase)
{
	int count = 0;

	int length = phrase.length();

	int curPos = 0;
	int tempPos;

	while (curPos < length)
	{
		while (curPos < length && phrase[curPos] == ' ')
			++curPos;

		if (curPos == length)
			break;

		++count;

		tempPos = curPos;

		while (tempPos < length - 1 && phrase[tempPos + 1] != ' ')
			++tempPos;

		curPos = tempPos + 1;
	}

	return count;
}

int printPos(int newsFile, fstream& paraPosFile, string query, int paraFilePos, int* startPos, posNumPair** hili)
{
	int paraNum = 0;

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)&paraNum, sizeof paraNum);
	paraPosFile.clear();

	// paraPos is 1-index
	int* paraPos = new int[paraNum + 2];

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)paraPos, (paraNum + 2) * (sizeof paraNum));
	paraPosFile.clear();

	//paraCount is 1-index
	int* paraCount = new int[paraNum + 1];
	for (int i = 0; i < paraNum + 1; ++i)
		paraCount[i] = 0;

	int length = query.length();

	int curPos = 0;
	int tempPos;

	string word;

	fixedStr k;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;

			word = query.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, word.c_str());

			if (word[0] == '+')
				word.erase(0, 1);

			if (word != "AND" && word != "OR" && (!isStopWord(k)))
			{
				if (word[0] == '~')
				{
					word.erase(0, 1);

					fixedStr k2;

					strcpy_s(k2.str, word.c_str());

					tokenPos = wordPositionsSyn(newsFile, k2, count);
				}
				else
				{
					bool ok = true;
					size_t found_1 = word.find('.'), found_2 = found_1 + 1;

					if (found_1 >= 0 && found_1 < word.length() - 1 && word[found_2] == '.')
					{
						for (int i = 0; i < found_1; i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						for (int i = found_2 + 1; i < word.length(); i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						if (ok == true)
						{
							string num1S = word.substr(0, found_1);
							string num2S = word.substr(found_2 + 1, word.length() - found_2 - 1);

							int num1 = stoi(num1S);
							int num2 = stoi(num2S);

							tokenPos = rangePositions(newsFile, num1, num2, count);
						}
						else
							tokenPos = wordPositions(newsFile, k, count);
					}
					else
						tokenPos = wordPositions(newsFile, k, count);
				}
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				++paraCount[paraInd];
				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	vector<paraScorePair> paraScore(paraNum);
	for (int i = 0; i < paraNum; ++i)
	{
		paraScore[i].para = i;
		paraScore[i].score = pow(paraCount[i + 1], 2) / (paraPos[i + 2] - paraPos[i + 1]);
	}

	sort(paraScore.begin(), paraScore.end(), comp1);

	int curPara = 0;

	while (curPara < 5 && curPara < paraNum && paraCount[paraScore[curPara].para + 1] != 0)
	{
		++curPara;
	}

	int outParaNum = curPara;

	sort(paraScore.begin(), paraScore.begin() + outParaNum, comp2);

	for (int i = 0; i < outParaNum; ++i)
	{
		startPos[i] = paraPos[paraScore[i].para + 1];
		hili[i] = new posNumPair[paraCount[paraScore[i].para + 1] + 1];
	}

	int hiliCur[5];
	for (int i = 0; i < 5; ++i)
		hiliCur[i] = 0;

	curPos = 0;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		int phraseLen = 1;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			phraseLen = phraseLength(phrase);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;

			word = query.substr(curPos, tempPos - curPos + 1);

			strcpy_s(k.str, word.c_str());

			if (word[0] == '+')
				word.erase(0, 1);

			if (word != "AND" && word != "OR" && (!isStopWord(k)))
			{
				if (word[0] == '~')
				{
					word.erase(0, 1);

					fixedStr k2;

					strcpy_s(k2.str, word.c_str());

					tokenPos = wordPositionsSyn(newsFile, k2, count);
				}
				else
				{
					bool ok = true;
					size_t found_1 = word.find('.'), found_2 = found_1 + 1;

					if (found_1 >= 0 && found_1 < word.length() - 1 && word[found_2] == '.')
					{
						for (int i = 0; i < found_1; i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						for (int i = found_2 + 1; i < word.length(); i++)
							if (!isNumber(word, i))
							{
								ok = false;
								break;
							}

						if (ok == true)
						{
							string num1S = word.substr(0, found_1);
							string num2S = word.substr(found_2 + 1, word.length() - found_2 - 1);

							int num1 = stoi(num1S);
							int num2 = stoi(num2S);

							tokenPos = rangePositions(newsFile, num1, num2, count);
						}
						else
							tokenPos = wordPositions(newsFile, k, count);
					}
					else
						tokenPos = wordPositions(newsFile, k, count);
				}
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;
		int curHiliFile = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				while (curHiliFile < outParaNum && paraScore[curHiliFile].para < paraInd - 1)
					++curHiliFile;

				if (curHiliFile == outParaNum)
					break;

				if (paraScore[curHiliFile].para == paraInd - 1)
				{
					int tempCur = hiliCur[curHiliFile];
					hili[curHiliFile][tempCur].pos = tokenPos[tokenInd];
					hili[curHiliFile][tempCur].wordNum = phraseLen;
					++hiliCur[curHiliFile];
				}

				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	for (int i = 0; i < outParaNum; ++i)
	{
		int tempCur = hiliCur[i];
		hili[i][tempCur].pos = -1;
	}

	delete[] paraPos;
	delete[] paraCount;

	return outParaNum;
}

/*int printPos(int newsFile, fstream& paraPosFile, string query, int paraFilePos, int* startPos, posNumPair** hili)
{
	int paraNum = 0;

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)&paraNum, sizeof paraNum);
	paraPosFile.clear();

	// paraPos is 1-index
	int* paraPos = new int[paraNum + 2];

	paraPosFile.seekg(paraFilePos, paraPosFile.beg);
	paraPosFile.read((char*)paraPos, (paraNum + 2) * (sizeof paraNum));
	paraPosFile.clear();

	//paraCount is 1-index
	int* paraCount = new int[paraNum + 1];
	for (int i = 0; i < paraNum + 1; ++i)
		paraCount[i] = 0;

	int length = query.length();

	int curPos = 0;
	int tempPos;

	string word;

	fixedStr k;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;

			word = query.substr(curPos, tempPos - curPos + 1);

			if (word != "AND" && word != "OR")
			{
				strcpy_s(k.str, word.c_str());

				tokenPos = wordPositions(newsFile, k, count);
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				++paraCount[paraInd];
				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	vector<paraScorePair> paraScore(paraNum);
	for (int i = 0; i < paraNum; ++i)
	{
		paraScore[i].para = i;
		paraScore[i].score = pow(paraCount[i + 1], 2) / (paraPos[i + 2] - paraPos[i + 1]);
	}

	sort(paraScore.begin(), paraScore.end(), comp1);

	int curPara = 0;

	while (curPara < 5 && curPara < paraNum && paraCount[paraScore[curPara].para + 1] != 0)
	{
		++curPara;
	}

	int outParaNum = curPara;

	sort(paraScore.begin(), paraScore.begin() + outParaNum, comp2);

	for (int i = 0; i < outParaNum; ++i)
	{
		startPos[i] = paraPos[paraScore[i].para + 1];
		hili[i] = new posNumPair[paraCount[paraScore[i].para + 1] + 1];
	}

	int hiliCur[5];
	for (int i = 0; i < 5; ++i)
		hiliCur[i] = 0;

	curPos = 0;

	while (curPos < length)
	{
		while (curPos < length && isScoreDelimiter(query, curPos))
			++curPos;

		if (curPos == length)
			break;

		int* tokenPos;
		int count = 0;

		int phraseLen = 1;

		if ('"' == query[curPos])
		{
			tempPos = curPos + 1;

			while (tempPos < length && query[tempPos] != '"')
				++tempPos;

			if (tempPos == length)
				break;

			string phrase = query.substr(curPos + 1, tempPos - curPos - 1);

			phraseLen = phraseLength(phrase);

			tokenPos = positionsInNewsFile(newsFile, phrase, count);
		}
		else
		{
			tempPos = curPos;

			while (tempPos < length - 1 && !isScoreDelimiter(query, tempPos + 1))
				++tempPos;

			word = query.substr(curPos, tempPos - curPos + 1);

			if (word != "AND" && word != "OR")
			{
				strcpy_s(k.str, word.c_str());

				tokenPos = wordPositions(newsFile, k, count);
			}
			else
				tokenPos = NULL;
		}

		if (NULL == tokenPos)
		{
			curPos = tempPos + 1;
			continue;
		}

		int paraInd = 1;
		int tokenInd = 0;
		int curHiliFile = 0;

		while (paraInd <= paraNum && tokenInd < count)
		{
			if (tokenPos[tokenInd] < paraPos[paraInd + 1])
			{
				while (curHiliFile < outParaNum && paraScore[curHiliFile].para < paraInd - 1)
					++curHiliFile;

				if (curHiliFile == outParaNum)
					break;

				if (paraScore[curHiliFile].para == paraInd - 1)
				{
					int tempCur = hiliCur[curHiliFile];
					hili[curHiliFile][tempCur].pos = tokenPos[tokenInd];
					hili[curHiliFile][tempCur].wordNum = phraseLen;
					++hiliCur[curHiliFile];
				}

				++tokenInd;
			}
			else
				++paraInd;
		}

		delete[] tokenPos;

		curPos = tempPos + 1;
	}

	for (int i = 0; i < outParaNum; ++i)
	{
		int tempCur = hiliCur[i];
		hili[i][tempCur].pos = -1;
	}

	delete[] paraPos;
	delete[] paraCount;

	return outParaNum;
}*/

int highlightPositions(int newsFile, string query, int* startPos, posNumPair** hili)
{
	int rootInd;

	string metaFilePath = rootPath + "metaPara.txt";

	ifstream metaRead;
	metaRead.open(metaFilePath);
	if (!metaRead.is_open())
	{
		cout << "Can't open meta file to read" << endl;
		return 0;
	}

	metaRead >> rootInd;

	metaRead.close();

	string BTreePath = rootPath + "BTreeParagraph.txt";
	string paraPath = rootPath + "para.txt";

	fstream BTree, paraPosFile;

	BTree.open(BTreePath, fstream::binary | fstream::out | fstream::in);
	paraPosFile.open(paraPath, fstream::binary | fstream::out | fstream::in);

	int pos = numPos(rootInd, newsFile, BTree);

	int temp = printPos(newsFile, paraPosFile, query, pos, startPos, hili);

	BTree.close();
	paraPosFile.close();

	return temp;
}

//-------------------------------

bool isHighlightDelimeter(string text, int curPos)
{
	return(text[curPos] == ' ' || text[curPos] == 10 || text[curPos] == '|' || text[curPos] == ','
		|| text[curPos] == ':' || text[curPos] == '!' || text[curPos] == '.' || text[curPos] == ' '
		|| text[curPos] == '"' || text[curPos] == '\n' || text[curPos] == '-'
		|| text[curPos] == '(' || text[curPos] == '—' || text[curPos] == ')' || text[curPos] == '/'
		);
}

void delete_non_ascii(string& para)
{
	ifstream fin;
	ofstream fout;
	stringstream buffer;

	for (int i = 0; i < newsFileCount; ++i)
	{
		string path = getNewsPaths(i);

		fin.open(path);

		buffer.str("");
		buffer.clear();

		buffer << fin.rdbuf();

		fin.close();

		string para = buffer.str();

		for (int i = 0; i < para.length(); i++)
			if (para[i] == '`' || ((para[i] < 32 || para[i] > 126) && para[i] != '\n'))
				para[i] = ' ';

		fout.open(path, ios::trunc);

		fout << para;

		fout.close();
	}
}