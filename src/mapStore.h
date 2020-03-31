#ifndef MAPSTORE_H_INCLUDED
#define MAPSTORE_H_INCLUDED

#include <map>
#include <sstream>

class symbolTable
{
public:
    int offset;
    int sym_size = 4; // ������Ĭ�ϴ�СΪ4
    string nature; //���� PARA CONST VAR ARRAY
    string type; //���� int char MEMĬ������Ϊint
    //string value; //for const : char / int
};

class funcInfo //������Ϣ
{
public:
    int offset; // ����ƫ����
    int para_cnt = 0; //��������
    string func_type; //�������� ������ֵ����
    int reg_if_used[10] = {0,0,0,0,0,0,0,0,0,0}; //�˺����ڼĴ���ʹ�����
    vector<pair<string, symbolTable> > LOCAL_TABLE; //������������ id + info
};

extern vector<pair<string, symbolTable> > GLOBAL_TABLE; //ȫ�ֳ������� id + info
extern vector<pair<string, funcInfo> >FUNC_MAP; // ������(id + info)

void STORE_CONSTANTS(string id,string type,string value); //�洢���� CONST ȫ��/�ֲ�     �ֲ��Ŀ��Ժ�ȫ�ֵ�����
void STORE_VARIABLES(string id,string type); //�洢���� VAR
void STORE_ARRAY(string id,string type,string arr_size); //�洢���� ARRAY
void STORE_FUNC(string id,funcInfo info); //�洢����
void STORE_PARATABLE(string id,string type); //�洢�����Ĳ����� PARA
void STORE_RETURN_VALUE();
int FIND_IN_GLOBAL(string id); //��ȫ�ֱ� ��������
int FIND_IN_LOCAL(string id); //�ڵ�ǰ����������Ĳ����䳣������
int GET_FUNC_INDEX(string func_name);
int GET_SYM_INDEX(int func_index,string sym_name);
int GET_GLOBAL_SYM_INDEX(string sym_name);
#endif // MAPSTORE_H_INCLUDED
