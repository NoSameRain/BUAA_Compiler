#ifndef INTERCODE_H_INCLUDED
#define INTERCODE_H_INCLUDED
#include <map>
#include<stack>

extern vector < vector <string> > intercode; //存储中间代码 四元式
extern vector < string > mipscode; //mips
extern map < string, string > str_to_print; //需要在data段存储的字符串

void push_inter(vector < string > tmp); //存入中间变量vec
void gene_code(); //生成mips
void initial_data_block(); //设立data段，存储要输出的字符串
void assign_sta_handler(); //处理赋值语句
void assign_store_handler(); //将赋值语句的结果存入内存
void assign_global_store(string name,int flag); //将赋值语句的结果存入内存(对应全局变量)
void operand_handler(string name,string reg); //处理操作数 (操作数名称,要存入的临时寄存器)
void operand_id_handler(string name,string reg); //处理 id id[exp] 型操作数
void const_handler();
void var_handler();
void func_handler();
void printf_handler();
void scanf_handler();
void scanf_global_handler(); //什么时候输入全局变量：1、局部没找到 2、局部找到了同名id却是个const
string get_offset(string name);
string get_type(string name);
void print_exp(string name); //输出形参or寄存器
void print_v_c_arr(string name); //int/char var/array/const
string char_to_num(string v); //把字符串'a'中的字符提取出来转换为数字字符串
int get_array_name(string id);
void goto_handler(); //main / labelX
void call_handler();//传参,存入形参内存 + jal func
void jal_handler(string called_func_name); //在jal语句前后进行对临时寄存器的存取 并jal
void store_para_to_memory(); //把传参值存入形参对应内存
void return_handler();
void store_arr_index(); //计算出数组元素下标的值并存入寄存器$t1
void branch_handler(); //cmp id1 op id2
void relea_occu_reg(char num,int mark); //释放/占用 每个函数里的临时寄存器 t2~t9
void mark_reg_type(char num,int mark); //设置寄存器类型 0 int 1 char
void set_MEM_type(string name); //设置MEM类型
string reverse_op(string op);
#endif // INTERCODE_H_INCLUDED
