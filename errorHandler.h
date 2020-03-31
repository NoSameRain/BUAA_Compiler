#ifndef ERRORHANDLER_H_INCLUDED
#define ERRORHANDLER_H_INCLUDED

extern int lineNum; // which line in file now
extern int flag_read; // flag = 0 -- read ahead now / flag = 1 -- read now
extern int READ_N; //本次get到的sym距离上一个sym换了几行
extern int LINE_ADD; //同上
extern int LACK_PARENT;//少右括号时会多读一位

extern vector <string> errorVec; // 输出错误类型

void A_illIDNFR(char s); // 非法标识符
void A_illChar(char s); //非法字符
void A_unMatchQuates(); //引号不匹配
void B_reDefName();
void C_unDefName(int i,string id);
void D_E_valParaTableError(string id,vector <string> VPT); //函数参数个数、类型不匹配
void F_IllTypeInCondition(); //条件判断中出现不合法类型
void G_Return_Surplus();
void H_Return_Lack();
void H_Return_unMatch(string ret_name); //返回值类型不匹配
void I_ARRAY_INDEX_ERROR(string index_name); //数组下标应为整型表达式
void J_CHANGE_CONST_VALUE(string id); //改变常量的值
void K_SHOULD_BE_SEMICN(); //缺少分号
void L_LACK_RPARENT(); //缺少右小括号
void M_LACK_RBRACK(); //缺少右中括号
void N_LACK_WHILE(); //do while缺少while
void O_constDefTypeError(); //常量定义中=后面只能是整型或字符型常量
void error(string a);
void printError(string a);

#endif // ERRORHANDLER_H_INCLUDED
