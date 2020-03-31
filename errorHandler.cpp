#include "header.h"
#include "mapStore.h"
#include "errorHandler.h"

vector <string> errorVec; // �����������

void A_illIDNFR(char s) // �Ƿ���ʶ��
{
     if(isdigit(s)) printError("a");
}

void A_illChar(char s) //�Ƿ��ַ�
{
    if(!isdigit(s) && !isalpha(s)
               && !isspace(s) && s != '+'
               && s != '-' && s != '*'
               && s != '/' && s != '_') {
                printError("a");
    }
}

void A_unMatchQuates() //���Ų�ƥ��   ˫���ŵĲ���д����������
{
printError("a");
}

void B_reDefName() //�ض�������
{
    printError("b");
}

void C_unDefName(int n,string id) // δ��������   0:const var array  1:func
{
    if(n == 0) {
        if(FIND_IN_LOCAL(id) == -1) {
            if(FIND_IN_GLOBAL(id) == 0){
                printError("c");

            }
        }
    } else {
        if(GET_FUNC_INDEX(id) == -1) {
            printError("c");
        }
    }
}

void D_E_valParaTableError(string id,vector <string> VPT) //�����������������Ͳ�ƥ��
{
    string name;
    string type_para;
    int flag;
    int called_func_index = GET_FUNC_INDEX(id);
    int para_cnt = FUNC_MAP[called_func_index].second.para_cnt; //�����ú�����������

    if(VPT.size() != para_cnt) printError("d"); //��������������ƥ��
    else {
        if(VPT.size() != 0) {
            for(int i = 0;i < para_cnt;i++) {
                type_para = FUNC_MAP[called_func_index].second.LOCAL_TABLE[i].second.type;
                name = VPT[i];
                flag = if_is_char(name); //$tx MEM id num 'a' �����map��˵��Ϊchar

                if(type_para == "char") {
                    if(flag == 0) {
                        printError("e"); //�����������Ͳ�ƥ��
                        break;
                    }
                } else {
                    if(flag == 1) {
                        printError("e"); //�����������Ͳ�ƥ��
                        break;
                    }
                }

            }
        }
    }
}

void F_IllTypeInCondition() //�����ж��г��ֲ��Ϸ�����
{
    printError("f");
}

void G_Return_Surplus()
{
    printError("g");
}

void H_Return_Lack()
{
    printError("h");

}

void H_Return_unMatch(string ret_name) //����ֵ���Ͳ�ƥ��
{
    string type = FUNC_MAP.back().second.func_type; //���غ�������
    int flag = if_is_char(ret_name); //$tx MEM id num 'a' �����map��˵��Ϊchar
    if(type == "char"){
        if(flag == 0) printError("h");
    } else {
        if(flag == 1) printError("h");
    }
}

void I_ARRAY_INDEX_ERROR(string index_name) //�����±�ӦΪ���ͱ��ʽ
{
    int flag = if_is_char(index_name); //$tx MEM id num 'a' �����map��˵��Ϊchar
    if(flag == 1) printError("i");
}

void J_CHANGE_CONST_VALUE(string id) //�ı䳣����ֵ
{
    int index = FIND_IN_LOCAL(id);
    string nature;
    if(index == -1) {
        index = GET_GLOBAL_SYM_INDEX(id);
        if(index != -1) {
            nature = GLOBAL_TABLE[index].second.nature;
            if(nature == "CONST") printError("j");
        }
    } else {
        nature = FUNC_MAP.back().second.LOCAL_TABLE[index].second.nature;
        if(nature == "CONST") printError("j");
    }

}

void K_SHOULD_BE_SEMICN() //ȱ�ٷֺ�
{
    //LINE_ADD:����get����sym������һ��sym���˼���
    lineNum = lineNum - LINE_ADD;
    printError("k");

    lineNum = lineNum + LINE_ADD;

}

void L_LACK_RPARENT() //ȱ����С����
{
    //LINE_ADD:����get����sym������һ��sym���˼���
    //cout<<s_word<<endl;
    lineNum = lineNum - LINE_ADD;
    printError("l");
    lineNum = lineNum + LINE_ADD;
}

void M_LACK_RBRACK() //ȱ����������
{
    //LINE_ADD:����get����sym������һ��sym���˼���
    lineNum = lineNum - LINE_ADD;
    printError("m");
    lineNum = lineNum + LINE_ADD;
}

void N_LACK_WHILE() //do whileȱ��while
{
    printError("n");
}


void O_constDefTypeError() //����������=����ֻ�������ͻ��ַ��ͳ���
{
    printError("o");
}

void printError(string a) //׼�����
{
    string space = " ";
    string n = std::to_string(lineNum);
    string s = n + space + a;
    errorVec.push_back(s);
    //cout<<errorVec.back()<<endl;
}

void error(string s)
{
    //cout<<s<<endl;
}
