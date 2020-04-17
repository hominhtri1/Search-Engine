#ifndef _POLISH_H_
#define _POLISH_H_

#include "Header.h"

using std::string;

template <typename Type>
class Node
{
public:
	Type val;
	Node *next;
	Node<Type>()
	{
		next = NULL;
	}

	Node<Type>(Type x)
	{
		val = x;
		next = NULL;
	}
};

template <typename Type>
class STACK
{
private:
	Node<Type> *head = NULL;
public:
	void push_back(Type x);
	Type pop_back();
	Type top();
	bool isEmpty();

	~STACK();
};
//-------------------------------------------------------
template<typename Type>
STACK<Type>::~STACK()
{
	while (head != NULL)
	{
		Node<Type> *cur = head;
		head = head->next;
		delete cur;
	}
}
//-------------------------------------------------------
template<typename Type>
Type STACK<Type>::top()
{
	if (!isEmpty())
		return head->val;

	throw "OUT OF STACK ERROR";
}
//-------------------------------------------------------
template <typename Type>
void STACK<Type>::push_back(Type x)
{
	if (head == NULL)
	{
		head = new Node<Type>(x);
	}
	else
	{
		Node<Type> *cur = new Node<Type>(x);
		cur->next = head;
		head = cur;
	}
}
//-------------------------------------------------------
template<typename Type>
Type STACK<Type>::pop_back()
{
	if (isEmpty())
	{
		throw "empty_stack_error";
	}
	Node<Type> *cur = head;
	head = head->next;
	Type val = cur->val;
	delete cur;
	return val;
}
//-------------------------------------------------------
template<typename Type>
bool STACK<Type>::isEmpty()
{
	return (head == NULL);
}
//-------------------------------------------------------
std::string getSub(std::string &str);
std::string postfixForm(std::string str);
void init(std::string &str);
int resolveInput(std::string str, bool* exist);

#endif