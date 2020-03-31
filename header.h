#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <algorithm>
#include <map>

#define MAX_code 1000000
#define MAX_word 100

using namespace std;

extern int codeN,ind;
extern int cateInd;
extern int cate; // cateInd 的预读
extern char code[MAX_code];
extern char g_word[MAX_word]; //预读单词
extern string s_word; //目前认可的单词
extern string callName;

const string cateCode[] = {
    "IDENFR","INTCON","CHARCON","STRCON","CONSTTK","INTTK", //0-5
    "CHARTK","VOIDTK","MAINTK","IFTK","ELSETK","DOTK", //6-11
    "WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","PLUS", //12-17
    "MINU","MULT","DIV","LSS","LEQ","GRE", //18-23
    "GEQ","EQL","NEQ","ASSIGN","SEMICN","COMMA", //24-29
    "LPARENT","RPARENT","LBRACK","RBRACK","LBRACE","RBRACE"
};

const string reserWord[] = {
    "const","int","char","void","main","if",
    "else","do","while","for","scanf","printf",
    "return"
};

extern vector <int> re_Track; // 每调用一次getsym,计算一次ind增长量，存进vec
extern map<string, int> char_map; //存储为char类型的变量 寄存器 MEM

void jLexical();
void JudgeReserWord();
void initial();
void jSyntax();
void getSym();
void constantSpeci();
void variableSpeci();
void ReturnFunc();
void noReturnFunc();
void mainFunc();
void constantDefin();
void variableDefin();
void declaHeader();
void paraTable(); //参数表
void compoundStatemnet(); //复合语句
void statementCol(); //语句列
void jStatement(); //语句
void ifSta(); //条件语句
void condition(); //条件
int relationOp(); //关系运算符
void loopSta(); //循环语句
void call_Assi_Sta(string gw); //调用及赋值语句
void assignSta(string gw); //赋值语句
void scanfSta(); //读语句
void PrintfSta(); //写语句
void returnSta(); //返回语句
string expression(); //表达式
string term(); //项
string factor(); //因子
void step(); //步长
void valParaTable(string gw); //值参数表
string jInteger(); //整数
int unsignInteger(); //无符号整数
int jString(); //字符串
void typeIdenti(); //类型标识符
void pushPrinVec(string str1);
int if_is_char(string name); //$tx MEM id num 'a' 如果在map里说明为char
#endif // HEADER_H_INCLUDED
