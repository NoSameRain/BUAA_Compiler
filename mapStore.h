#ifndef MAPSTORE_H_INCLUDED
#define MAPSTORE_H_INCLUDED

#include <map>
#include <sstream>

class symbolTable
{
public:
    int offset;
    int sym_size = 4; // 除数组默认大小为4
    string nature; //属性 PARA CONST VAR ARRAY
    string type; //类型 int char MEM默认类型为int
    //string value; //for const : char / int
};

class funcInfo //函数信息
{
public:
    int offset; // 函数偏移量
    int para_cnt = 0; //参数个数
    string func_type; //函数类型 即返回值类型
    int reg_if_used[10] = {0,0,0,0,0,0,0,0,0,0}; //此函数内寄存器使用情况
    vector<pair<string, symbolTable> > LOCAL_TABLE; //常变量参数表 id + info
};

extern vector<pair<string, symbolTable> > GLOBAL_TABLE; //全局常变量表 id + info
extern vector<pair<string, funcInfo> >FUNC_MAP; // 函数表(id + info)

void STORE_CONSTANTS(string id,string type,string value); //存储常量 CONST 全局/局部     局部的可以和全局的重名
void STORE_VARIABLES(string id,string type); //存储变量 VAR
void STORE_ARRAY(string id,string type,string arr_size); //存储数组 ARRAY
void STORE_FUNC(string id,funcInfo info); //存储函数
void STORE_PARATABLE(string id,string type); //存储函数的参数表 PARA
void STORE_RETURN_VALUE();
int FIND_IN_GLOBAL(string id); //在全局变 常量中找
int FIND_IN_LOCAL(string id); //在当前函数作用域的参数变常量里找
int GET_FUNC_INDEX(string func_name);
int GET_SYM_INDEX(int func_index,string sym_name);
int GET_GLOBAL_SYM_INDEX(string sym_name);
#endif // MAPSTORE_H_INCLUDED
