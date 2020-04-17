#include "Polish.h"
#include "BTree.h"
#include "STree.h"
#include "BTreeStopWord.h"
#include "Synonym.h"

//--------------------------------------------------
bool isKeyWord(std::string &str)
{
	//Nguyen Vo Duc Loc
	return (str == "OR" || str == "AND" || str == " " || str == "NOT");
}
//--------------------------------------------------
bool isOperator(std::string &str)
{
	return (str == ")" || str == "(");
}
//--------------------------------------------------
void init(std::string &str)
{
	//Nguyen Vo Duc Loc
	//xoa ki ty trang du thua

	int i = 0;
	while (i < str.length())
	{
		if (str[i] == ' ')
		{
			if (i == 0 || i == str.length() - 1 || (i < str.length() - 1 && str[i + 1] == ' '))
			{
				str.erase(i, 1);
			}
			else
				i++;
		}
		else
		{
			i++;
		}
	}
}
//--------------------------------------------------
std::string getSub(std::string &str)
{
	//Nguyen Vo Duc Loc
	//chia string thanh tung part de xu ly
	//ex: str = "ong tri khung" => getSub = "ong", str = "tri khung"

	int i = 0;
	std::string chr = "";

	if (str[0] == '(' || str[0] == ')')
	{
		chr = str[0];
	}
	else
	{
		int ok = 1;
		while (i < str.length() && str[i] != ')' && str[i] != '(' && (str[i] != ' ' || ok < 0))
		{
			if (str[i] == '"')
				ok = -ok;
			chr = chr + str[i];
			i++;
		}
	}

	if (i > 0 && i < str.length() && str[i] == ')')
		str.erase(0, i);
	else
		str.erase(0, i + 1);

	if (str != "" && str[0] == ' ')
		str.erase(0, 1);

	if (!isOperator(chr) && !isKeyWord(chr) && chr[0] != '"')
		chr = '"' + chr + '"';

	return chr;
}
//-------------------------------------------------
std::string postfixForm(std::string str)
{
	//Nguyen Vo Duc Loc
	//dang postfix cua str

	STACK<std::string> BOX;

	std::string expression = "";

	std::string sub = "", pre_sub = "";
	while (str != "")
	{
		sub = getSub(str);
		if (isKeyWord(sub))
		{
			if (BOX.isEmpty())
				BOX.push_back(sub);
			else
			{
				while (!BOX.isEmpty() && BOX.top() != "(")
				{
					expression = expression + ' ' + BOX.pop_back();
				}
				BOX.push_back(sub);
			}
		}
		else
			if (isOperator(sub))
			{
				if (sub == "(")
				{
					if (pre_sub == ")")
					{
						BOX.push_back("AND");
					}

					BOX.push_back(sub);
				}
				else
				{
					while (!BOX.isEmpty() && BOX.top() != "(")
					{
						expression = expression + ' ' + BOX.pop_back();
					}
					if (!BOX.isEmpty())
						BOX.pop_back();
				}
			}
			else
				if (!isKeyWord(pre_sub) && !isOperator(pre_sub) && pre_sub != "")
				{
					while (!BOX.isEmpty() && BOX.top() != "(")
					{
						expression = expression + ' ' + BOX.pop_back();
					}
					BOX.push_back("AND");
					expression = expression + ' ' + sub;
				}
				else
				{
					expression = expression + ' ' + sub;
				}

		pre_sub = sub;
	}

	while (!BOX.isEmpty())
	{
		expression = expression + ' ' + BOX.pop_back();
	}

	while (expression[0] == ' ')
		expression.erase(0, 1);

	return expression;
}
//-------------------------------------------------
bool isWordInFile(const std::string &word, const std::string &source)
{
	std::ifstream fi;
	fi.open(source, std::ios_base::binary);

	if (!fi.is_open())
	{
		std::cout << "can't open file " << source << std::endl;
		return false;
	}

	bool ok = false;
	std::string str;
	while (!fi.eof())
	{
		getline(fi, str, ' ');
		if (str == word)
		{
			ok = true;
			break;
		}
	}

	return ok;
	fi.close();
}
//-------------------------------------------------
std::string nameFile(int i)
{
	std::string str = "";

	int tmp = i;
	do
	{
		str = char(i % 10 + '0') + str;
		i = i / 10;
	} while (i > 0);

	if (tmp < 10)
		str = "0" + str;

	return (rootPath + "News files\\Group05News" + str + ".txt");
}
//-------------------------------------------------

bool isInitial(bool *&Arr)
{
	int count = 0;
	for (int i = 0; i < newsFileCount; i++)
	{
		if (Arr[i] == 1)
			count++;
	}
	if (count == 0)
		return true;
	return false;
}
//-------------------------------------------------

// Add file name variable

/*bool *setArray(const std::string &str)
{
	bool *Arr = new bool[100];

	for (int i = 0; i < 100; i++)
		Arr[i] = false;

	for (int i = 75; i < 86; i++)
		if (isWordInFile(str, nameFile(i)))
			Arr[i] = true;

	return Arr;
}*/

// Add file name variable

bool* setArray(string str)
{
	int i = 0;

	while (i < str.length() && str[i] != ' ' && str[i] != '*')
		++i;

	bool* exist = new bool[newsFileCount];

	if (i < str.length())
	{
//		for (int j = 0; j < newsFileCount; ++j)
//			exist[j] = existInNewsFileSTree(j, str);
		existInAllNewsFileSTree(str, exist);
	}
	else
	{
		for (int j = 0; j < newsFileCount; ++j)
			exist[j] = false;

		if (str[0] == '-')
		{
			str.erase(0, 1);

			fixedStr k;

			strcpy_s(k.str, str.c_str());

			existInNewsFileBTree(k, exist);

			for (int j = 0; j < newsFileCount; ++j)
				exist[j] = !exist[j];
		}
		else if (str[0] == '+')
		{
			str.erase(0, 1);

			fixedStr k;

			strcpy_s(k.str, str.c_str());
			
			if (isStopWord(k))
			{
				for (int j = 0; j < newsFileCount; ++j)
					exist[j] = true;
			}
			else
				existInNewsFileBTree(k, exist);
		} 
		else if (str[0] == '~')
		{
			str.erase(0, 1);

			fixedStr k;

			strcpy_s(k.str, str.c_str());
			
			existInNewsFileBTreeSyn(k, exist);
		}
		else if (str.substr(0, 8) == "intitle:")
		{
			str.erase(0, 8);
			
			fixedStr k;

			strcpy_s(k.str, str.c_str());

			existInNewsFileTitleBTree(k, exist);
		}
		else if (str.substr(0, 9) == "filetype:")
		{
			for (int j = 0; j < newsFileCount; ++j)
				exist[j] = true;
		}
		else
		{
			bool ok = true;
			size_t found_1 = str.find('.'), found_2 = found_1 + 1;

			if (found_1 >= 0 && found_1 < str.length() - 1 && str[found_2] == '.')
			{
				for (int i = 0; i < found_1; i++)
					if (!isNumber(str, i))
					{
						ok = false;
						break;
					}

				for (int i = found_2 + 1; i < str.length(); i++)
					if (!isNumber(str, i))
					{
						ok = false;
						break;
					}

				if (ok == true)
				{
					string num1S = str.substr(0, found_1);
					string num2S = str.substr(found_2 + 1, str.length() - found_2 - 1);

					int num1 = stoi(num1S);
					int num2 = stoi(num2S);

					rangeExist(num1, num2, exist);
				}
				else
				{
					fixedStr k;

					strcpy_s(k.str, str.c_str());

					if (isStopWord(k))
					{
						for (int j = 0; j < newsFileCount; ++j)
							exist[j] = true;
					}
					else
						existInNewsFileBTree(k, exist);
				}
			}
			else
			{
				fixedStr k;

				strcpy_s(k.str, str.c_str());

				if (isStopWord(k))
				{
					for (int j = 0; j < newsFileCount; ++j)
						exist[j] = true;
				}
				else
					existInNewsFileBTree(k, exist);
			}
		}
	}

	return exist;
}

//-------------------------------------------------
void generate(bool *Arr_1, bool *Arr_2, const std::string &key)
{
	if (key == "AND")
	{
		for (int i = 0; i < 100; i++)
			Arr_1[i] = Arr_1[i] && Arr_2[i];
	}
	else
		if (key == "OR")
		{
			for (int i = 0; i < 100; i++)
				Arr_1[i] = Arr_1[i] || Arr_2[i];
		}
		else
			if (key == "NOT")
			{
				for (int i = 0; i < 100; i++)
					Arr_1[i] = Arr_1[i] && !Arr_2[i];
			}
}
//-------------------------------------------------
int resolveInput(std::string str, bool* exist)
{
	//Nguyen Vo Duc Loc
	//this is the main function

	STACK<bool*> BOX;
	std::string expression = postfixForm(str), sub = "";
//	std::cout << expression << std::endl << std::endl;

	while (expression != "")
	{
		sub = getSub(expression);
		if (!isKeyWord(sub))
		{
			sub.erase(0, 1);
			sub.erase(sub.length() - 1, 1);

			BOX.push_back(setArray(sub));
		}
		else
		{
			bool *Arr_2 = BOX.pop_back();
			generate(BOX.top(), Arr_2, sub);
			delete[] Arr_2;
		}
	}

	bool *result = BOX.pop_back();

	for (int i = 0; i < newsFileCount; i++)
		exist[i] = false;

	int count = 0;

	if (result != NULL)
	{
		for (int i = 0; i < newsFileCount; i++)
		{
			if (result[i])
			{
				exist[i] = true;
				++count;
			}
		}
	}

	delete[] result;
	
	return count;
}