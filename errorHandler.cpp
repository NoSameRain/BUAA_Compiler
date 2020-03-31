#include "header.h"
#include "mapStore.h"
#include "errorHandler.h"

vector <string> errorVec; // 输出错误类型

void A_illIDNFR(char s) // 非法标识符
{
     if(isdigit(s)) printError("a");
}

void A_illChar(char s) //非法字符
{
    if(!isdigit(s) && !isalpha(s)
               && !isspace(s) && s != '+'
               && s != '-' && s != '*'
               && s != '/' && s != '_') {
                printError("a");
    }
}

void A_unMatchQuates() //引号不匹配   双引号的不会写！！！！！
{
printError("a");
}

void B_reDefName() //重定义名字
{
    printError("b");
}

void C_unDefName(int n,string id) // 未定义名字   0:const var array  1:func
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

void D_E_valParaTableError(string id,vector <string> VPT) //函数参数个数、类型不匹配
{
    string name;
    string type_para;
    int flag;
    int called_func_index = GET_FUNC_INDEX(id);
    int para_cnt = FUNC_MAP[called_func_index].second.para_cnt; //被调用函数参数个数

    if(VPT.size() != para_cnt) printError("d"); //函数参数个数不匹配
    else {
        if(VPT.size() != 0) {
            for(int i = 0;i < para_cnt;i++) {
                type_para = FUNC_MAP[called_func_index].second.LOCAL_TABLE[i].second.type;
                name = VPT[i];
                flag = if_is_char(name); //$tx MEM id num 'a' 如果在map里说明为char

                if(type_para == "char") {
                    if(flag == 0) {
                        printError("e"); //函数参数类型不匹配
                        break;
                    }
                } else {
                    if(flag == 1) {
                        printError("e"); //函数参数类型不匹配
                        break;
                    }
                }

            }
        }
    }
}

void F_IllTypeInCondition() //条件判断中出现不合法类型
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

void H_Return_unMatch(string ret_name) //返回值类型不匹配
{
    string type = FUNC_MAP.back().second.func_type; //返回函数类型
    int flag = if_is_char(ret_name); //$tx MEM id num 'a' 如果在map里说明为char
    if(type == "char"){
        if(flag == 0) printError("h");
    } else {
        if(flag == 1) printError("h");
    }
}

void I_ARRAY_INDEX_ERROR(string index_name) //数组下标应为整型表达式
{
    int flag = if_is_char(index_name); //$tx MEM id num 'a' 如果在map里说明为char
    if(flag == 1) printError("i");
}

void J_CHANGE_CONST_VALUE(string id) //改变常量的值
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

void K_SHOULD_BE_SEMICN() //缺少分号
{
    //LINE_ADD:本次get到的sym距离上一个sym换了几行
    lineNum = lineNum - LINE_ADD;
    printError("k");

    lineNum = lineNum + LINE_ADD;

}

void L_LACK_RPARENT() //缺少右小括号
{
    //LINE_ADD:本次get到的sym距离上一个sym换了几行
    //cout<<s_word<<endl;
    lineNum = lineNum - LINE_ADD;
    printError("l");
    lineNum = lineNum + LINE_ADD;
}

void M_LACK_RBRACK() //缺少右中括号
{
    //LINE_ADD:本次get到的sym距离上一个sym换了几行
    lineNum = lineNum - LINE_ADD;
    printError("m");
    lineNum = lineNum + LINE_ADD;
}

void N_LACK_WHILE() //do while缺少while
{
    printError("n");
}


void O_constDefTypeError() //常量定义中=后面只能是整型或字符型常量
{
    printError("o");
}

void printError(string a) //准备输出
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
