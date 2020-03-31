#ifndef INTERCODE_H_INCLUDED
#define INTERCODE_H_INCLUDED
#include <map>
#include<stack>

extern vector < vector <string> > intercode; //�洢�м���� ��Ԫʽ
extern vector < string > mipscode; //mips
extern map < string, string > str_to_print; //��Ҫ��data�δ洢���ַ���

void push_inter(vector < string > tmp); //�����м����vec
void gene_code(); //����mips
void initial_data_block(); //����data�Σ��洢Ҫ������ַ���
void assign_sta_handler(); //����ֵ���
void assign_store_handler(); //����ֵ���Ľ�������ڴ�
void assign_global_store(string name,int flag); //����ֵ���Ľ�������ڴ�(��Ӧȫ�ֱ���)
void operand_handler(string name,string reg); //��������� (����������,Ҫ�������ʱ�Ĵ���)
void operand_id_handler(string name,string reg); //���� id id[exp] �Ͳ�����
void const_handler();
void var_handler();
void func_handler();
void printf_handler();
void scanf_handler();
void scanf_global_handler(); //ʲôʱ������ȫ�ֱ�����1���ֲ�û�ҵ� 2���ֲ��ҵ���ͬ��idȴ�Ǹ�const
string get_offset(string name);
string get_type(string name);
void print_exp(string name); //����β�or�Ĵ���
void print_v_c_arr(string name); //int/char var/array/const
string char_to_num(string v); //���ַ���'a'�е��ַ���ȡ����ת��Ϊ�����ַ���
int get_array_name(string id);
void goto_handler(); //main / labelX
void call_handler();//����,�����β��ڴ� + jal func
void jal_handler(string called_func_name); //��jal���ǰ����ж���ʱ�Ĵ����Ĵ�ȡ ��jal
void store_para_to_memory(); //�Ѵ���ֵ�����βζ�Ӧ�ڴ�
void return_handler();
void store_arr_index(); //���������Ԫ���±��ֵ������Ĵ���$t1
void branch_handler(); //cmp id1 op id2
void relea_occu_reg(char num,int mark); //�ͷ�/ռ�� ÿ�����������ʱ�Ĵ��� t2~t9
void mark_reg_type(char num,int mark); //���üĴ������� 0 int 1 char
void set_MEM_type(string name); //����MEM����
string reverse_op(string op);
#endif // INTERCODE_H_INCLUDED
