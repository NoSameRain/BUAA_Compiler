#include "header.h"
#include "interCode.h"
#include "mapStore.h"
int sp_off = 6000;
int sp = 0;
int flag_reg_type = 0; //��¼�üĴ��������Ƿ�Ϊchar
int i_inter; //intercode[x][y]��x�±�
int reg_type[10]; //�Ĵ����洢ֵ������ 0 int 1 char
string array_name = "a";
string array_index = "b";
string a0,v0; //�Ĵ���
string tmp0 = "$t0";
string tmp1 = "$t1";
string type_int = "int";
string type_char = "char";
string type_void = "void";
string nature_arr = "ARRAY"; //PARA CONST VAR ARRAY
string nature_const = "CONST";
vector <vector <string> > intercode; //�洢�м���� ��Ԫʽ
vector <string> mipscode; //mips
stack <int> p_func; //�洢��ǰ�����ĺ�����func_map����±�
stack <string> p_para; //���ݲ���
map <string,int> MEM_type; //��¼MEM������

void push_inter(vector <string> tmp) //�����м����vec
{
    intercode.push_back(tmp);
}

void gene_code() //����mips
{
    mipscode.push_back(".data");

    for(i_inter = 0;i_inter < intercode.size();i_inter++) {
        if(intercode[i_inter][0] == "const") {
            const_handler();
        } else if(intercode[i_inter][0] == "var") {
            if(p_func.empty()) var_handler(); //ȫ�ֱ�������label�����ڴ� �ֲ��Ĳ���
        } else if(intercode[i_inter][0] == "func") {
            func_handler();   //����ѭ��
        } else if(intercode[i_inter][0] == "para") {
            // �;ֲ�����һ�����ùܰ�
        } else if(intercode[i_inter][0] == "printf") {
           printf_handler();
        } else if(intercode[i_inter][0] == "scanf") {
            scanf_handler();
        } else if(intercode[i_inter][0] == "cmp") {
            branch_handler(); //cmp id1 op id2
        } else if(intercode[i_inter][0] == "GOTO") { // j labelX
            goto_handler();
        } else if(intercode[i_inter][0] == "push") {
            p_para.push(intercode[i_inter][1]);
            while(intercode[i_inter + 1][0] == "push") {
                i_inter++;
                p_para.push(intercode[i_inter][1]);
            }
        } else if(intercode[i_inter][0] == "call") {
            call_handler();
        } else if(intercode[i_inter][0] == "ret") { // ret + $tx (����array) /  id  /  num  /  ��a��
            return_handler();
        } else if(intercode[i_inter][1] == ":") { //label :
            string str1 = intercode[i_inter][0] + intercode[i_inter][1];
            mipscode.push_back(str1);
        }else { //id : $t a a[exp] 'c' ����������ֵ!
            assign_sta_handler();
        }
    }
    string str2 = "addi $s0,$s0," + to_string(sp_off);
    mipscode.push_back(str2); //�ͷ�s0
    mipscode.push_back("li $v0,10");
    mipscode.push_back("syscall");
}

void const_handler() //int / char
{
    int sym_index;
    string sym_type;
    string id = intercode[i_inter][2];
    string v = intercode[i_inter][4]; //value

    if(!p_func.empty()){ // �ֲ����븳ֵ���ʽͬ��
        int func_index = p_func.top(); //��ǰ�����±�
        sym_index =  GET_SYM_INDEX(func_index,id);
        if(sym_index != -1) sym_type = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type;


        if (sym_type == type_char) {
            v = char_to_num(v); //���ַ���'a'�е��ַ���ȡ����ת��Ϊ�����ַ���
        }

        string str1 = "addi $t0,$0," + v; //addi $t0,$0,imm
        mipscode.push_back(str1);
        //sw $t0,offset($s0)
        string str2 = "sw $t0," + get_offset(id) + "($s0)";
        mipscode.push_back(str2);
    } else { //ȫ�ֳ�������label�����ڴ�
        string str1;

        sym_index = GET_GLOBAL_SYM_INDEX(id);
        if(sym_index != -1) sym_type = GLOBAL_TABLE[sym_index].second.type;

        if (sym_type == type_char) {
            v = char_to_num(v); //���ַ���'a'�е��ַ���ȡ����ת��Ϊ�����ַ���
        }
        //label: .word value
        str1 = id + ": .word " + v;
        mipscode.push_back(str1);
    }
}

void var_handler() //int char array ֻ����ȫ�ֵ�
{
    string id = intercode[i_inter][2];
    string sym_size;

    if(get_array_name(id)) { //Ϊ����id
        int sym_index = GET_GLOBAL_SYM_INDEX(array_name);
        if(sym_index != -1) sym_size = to_string(GLOBAL_TABLE[sym_index].second.sym_size);
        id = array_name;
    }  else {
        sym_size = "4";
    }

    string str1 = id + ": .space " + sym_size;
    mipscode.push_back(str1);

}

void func_handler()
{
    string func_name = intercode[i_inter][2];
    string str_mips = func_name + ":";
    int func_index = GET_FUNC_INDEX(func_name);

    if(p_func.empty()) {
        if(!str_to_print.empty()){
           map<string,string> ::iterator it;
            for(it=str_to_print.begin();it!=str_to_print.end();it++) {
                string str1 = it->first + ": .asciiz " + "\"" + it->second + "\"";
            mipscode.push_back(str1);
            }
        }

        mipscode.push_back("enter: .asciiz \"\\n\"");
        mipscode.push_back(".text");
        string str2 = "subi $s0,$sp," + to_string(sp_off);
        mipscode.push_back(str2);
        mipscode.push_back("j main");
    } else {
        //������һ��sp_func
        p_func.pop();

        mipscode.push_back("lw $ra,0($sp)");
        mipscode.push_back("addi $sp,$sp,4");
        mipscode.push_back("jr $ra");
    }

    p_func.push(func_index);
    mipscode.push_back(str_mips); //func :
    if(func_name != "main") {
        mipscode.push_back("addi $sp,$sp,-4");
        mipscode.push_back("sw $ra,0($sp)"); //һ����һ���º����͸Ͻ���һ��ra
    }
}

void printf_handler() //int char
{
    string name = intercode[i_inter][1];
    map<string,string> ::iterator iter = str_to_print.find(name); //find STRX
    if(iter != str_to_print.end()) { //���STR-X �ַ���
        string label = iter->first;
        //la $a0,label
        string str1 = "la $a0," + label;
        mipscode.push_back(str1);
        //li $v0,4
        mipscode.push_back("li $v0,4");
        //syscall
        mipscode.push_back("syscall");
        if(intercode[i_inter].size() == 5) print_exp(intercode[i_inter][4]);
        else { //����
            //la $a0,enter
            mipscode.push_back("la $a0,enter");
            //li $v0,4
            mipscode.push_back("li $v0,4");
            //syscall
            mipscode.push_back("syscall");
        }
    } else {   //����β�or�Ĵ���or'a' //��װ��ӡint
        print_exp(name);
    }
}

void scanf_global_handler() //ʲôʱ������ȫ�ֱ�����1���ֲ�û�ҵ� 2���ֲ��ҵ���ͬ��idȴ�Ǹ�const
{
    string name = intercode[i_inter][1];
    string type;
    int sym_index = GET_GLOBAL_SYM_INDEX(name); //��ȫ��
    if(sym_index != -1) type = GLOBAL_TABLE[sym_index].second.type;


    if (type == type_int) {
        //li $v0,5
        mipscode.push_back("li $v0,5");
    } else if (type == type_char) {
        //li $v0,12
        mipscode.push_back("li $v0,12");
    }
    mipscode.push_back("syscall");
    string str1 = "sw $v0," + name + "($0)";
    mipscode.push_back(str1);
}

void scanf_handler() //int char
{
    int offset;
    string str1,type,nature;
    string name = intercode[i_inter][1];

    int func_index = p_func.top(); //��ǰ�����±�
    int sym_index = GET_SYM_INDEX(func_index,name); //���Ҿֲ�

    if(sym_index == -1) { //not found
        scanf_global_handler(); //�ֲ�û�ҵ�
    } else { //found
        nature = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.nature;
        if(nature == nature_const) { //const
            scanf_global_handler(); //�ֲ��ҵ���ͬ��idȴ�Ǹ�const
        } else { //var
            offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;
            type = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type;
            if (type == type_int) {
                //li $v0,5
                mipscode.push_back("li $v0,5");
            } else if (type == type_char) {
                //li $v0,12
                mipscode.push_back("li $v0,12");
            }
            mipscode.push_back("syscall");
            //sw $v0,offset($s0)
            str1 = "sw $v0," + to_string(offset) +"($s0)";
            mipscode.push_back(str1);
       }
    }
}

void print_exp(string name) //����β�or�Ĵ���(array) int/char var
{
    int len = name.length();
    char c[len];
    string str1;
    strcpy(c, name.c_str());

    if(c[0] == '$') { //reg  int / char!!!
        str1 = "move $a0," + name;
        mipscode.push_back(str1);
        int num = c[2] - 48;
        if(reg_type[num] == 1) { //char
            mipscode.push_back("li $v0,11");
        } else { //int
            mipscode.push_back("li $v0,1");
        }
        mipscode.push_back("syscall");
        relea_occu_reg(c[2],0); //�ͷŸú��������ʱ�Ĵ��� t2~t9
        mark_reg_type(c[2],0); //��ʼ���üĴ�������Ϊint
    } else if (c[0] == '\'') { // char
        string char_n = char_to_num(name);
        str1 = "li $a0," + char_n;
        mipscode.push_back(str1);
        mipscode.push_back("li $v0,11");
        mipscode.push_back("syscall");
    } else { // int/char : var const para
        print_v_c_arr(name);
    }

    //����
    mipscode.push_back("la $a0,enter");
    mipscode.push_back("li $v0,4");
    mipscode.push_back("syscall");
}

void print_v_c_arr(string name) //int/char var/const
{
    string str1,type;
    int sym_index,func_index,offset;

    func_index = p_func.top(); //��ǰ�����±�
    sym_index = GET_SYM_INDEX(func_index,name); //���Ҿֲ�

    if(sym_index == -1) { //global
        sym_index = GET_GLOBAL_SYM_INDEX(name); //�õ�ָ����ȫ��id
        if(sym_index != -1) type = GLOBAL_TABLE[sym_index].second.type;
        //var
        //lw $a0,label($0)
        str1 = "lw $a0," + name + "($0)";

        mipscode.push_back(str1);
    } else { //local
        offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;
        type = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type;
        //var
        //lw $a0,offset($s0)
        str1 = "lw $a0," + to_string(offset) +"($s0)";
        mipscode.push_back(str1);
    }

    if(type == type_int) {
        mipscode.push_back("li $v0,1");
    } else if (type == type_char) {

        mipscode.push_back("li $v0,11");
    }
    mipscode.push_back("syscall");
}

void assign_sta_handler() //����ֵ��� a = b /a = b op c
{
    string name2,op;
    string name1 = intercode[i_inter][2];
    string result = intercode[i_inter][0]; //$tx id id[exp]

    if(intercode[i_inter].size() == 5){ //a = b op c
        name2 = intercode[i_inter][4];
        op = intercode[i_inter][3];
    } else { //a = b
        name2 = "0";
        op = "+";
    }

    int flag = 0;
    string str1,str2;
    int len3 = result.length();
    char c3[len3];
    strcpy(c3, result.c_str());
    if(c3[0] == '$'){ //$tx Ϊ��ʱ�Ĵ���
        flag = 1;
        relea_occu_reg(c3[2],1); //ռ�øú��������ʱ�Ĵ��� t2~t9
    }
    //tmp0 = $t0 / $0 / $tx

    operand_handler(name1,"$t0"); //��������� ��flag_reg_type��ֵ 0 int 1 char
    operand_handler(name2,"$t1"); //���������

    if(op == "+") { //add $t0,$t0,%t1
        if(flag) {
            //����$tx = arr[i]/ $tx = RET,$tx����Ϊchar�����
            if(intercode[i_inter].size() == 3 && flag_reg_type == 1) {
                mark_reg_type(c3[2],1); //�üĴ�������Ϊchar
                flag_reg_type = 0; //��ԭ
            }
            str1 = "add " + result + "," + tmp0 + "," + tmp1;
            mipscode.push_back(str1);
        } else {
            if(tmp0 == "$0") {
                str1 = "move $t0," + tmp1;
                tmp0 = "$t0";
            } else {
                str1 = "add " + tmp0 + "," + tmp0 + "," + tmp1;
            }
            mipscode.push_back(str1);

            if(intercode[i_inter].size() == 3 && flag_reg_type == 1)  {
                if(c3[0] == 'M' && c3[1] == 'E' && c3[2] =='M') {
                    set_MEM_type(result); //����MEM����
                }
            }
            assign_store_handler(); //����ֵ���Ľ�������ڴ�
        }
    } else if (op == "-") { //sub $t0,$t0,%t1
        if(flag) {
            str1 = "sub " + result + "," + tmp0 + "," + tmp1;
            mipscode.push_back(str1);
        } else {
            if(tmp0 == "$0") {
                str1 = "sub $t0," + tmp0 + "," + tmp1;
                tmp0 = "$t0";
            } else {
                str1 = "sub " + tmp0 + "," + tmp0 + "," + tmp1;
            }
            mipscode.push_back(str1);
            assign_store_handler(); //����ֵ���Ľ�������ڴ�
        }
    } else if (op == "*") { //mult $t0,%t1 //mflo $t0
        str1 = "mult " + tmp0 + "," + tmp1;
        mipscode.push_back(str1);
        if(flag) {
            str2 = "mflo " + result;
            mipscode.push_back(str2);
        } else {
            str2 = "mflo " + tmp0;
            mipscode.push_back(str2);
            assign_store_handler(); //����ֵ���Ľ�������ڴ�
        }
    } else if (op == "/") { //div $t0,%t1 //mflo $t0
        str1 = "div " + tmp0 + "," + tmp1;
        mipscode.push_back(str1);
        if(flag) {
            str2 = "mflo " + result;
            mipscode.push_back(str2);
        } else {
            str2 = "mflo " + tmp0;
            mipscode.push_back(str2);
            assign_store_handler(); //����ֵ���Ľ�������ڴ�
        }
    }
}

void assign_store_handler() //����ֵ���Ľ�������ڴ� ��Ҫ��� id id[exp] Ĭ�Ͼֲ���������һ������ֵ��������
{
    string str1,nature;
    string name = intercode[i_inter][0];
    int flag = get_array_name(name); //array judge
    if(flag) {
        name = array_name;
    }

    int func_index = p_func.top(); //��ǰ�����±�
    int sym_index = GET_SYM_INDEX(func_index,name); //���Ҿֲ�

    if(sym_index == -1) { //global
        assign_global_store(name,flag); //����ֵ���Ľ�������ڴ�(��Ӧȫ�ֱ���)
    } else { //local
        int offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;
        //nature = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.nature;
        //if(nature == nature_const) assign_global_store(name,flag); //����ֵ���Ľ�������ڴ�(��Ӧȫ�ֱ���)
        //else {
            if(flag) { //array
                store_arr_index(); //���������Ԫ���±�*4��ֵ������Ĵ���$t1
                //addi $t1,$t1,offset
                string str2 = "addi $t1,$t1," + to_string(offset);
                mipscode.push_back(str2);

                string str4 = "add $s0,$s0,$t1";
                mipscode.push_back(str4);

                str1 = "sw " + tmp0 + "," +"($s0)";
                mipscode.push_back(str1);

                string str5 = "sub $s0,$s0,$t1";
                mipscode.push_back(str5);
            } else { //var
                //sw $tx,offset($s0)
                str1 = "sw " + tmp0 + "," + to_string(offset) +"($s0)";
                mipscode.push_back(str1);
            }
        //}
    }
}

void assign_global_store(string name,int flag) //����ֵ���Ľ�������ڴ�(��Ӧȫ�ֱ���)
{
    //int sym_index = GET_GLOBAL_SYM_INDEX(name); //�õ�ָ����ȫ��id
    string str1;
    if(flag) { //array
            store_arr_index(); //���������Ԫ���±��ֵ������Ĵ���$t1
            //sw $t0,label($t1)
            str1 = "sw " + tmp0 + "," + name + "($t1)";
    } else { //var
            //sw $t0,label($0)
            str1 = "sw " + tmp0 + "," + name + "($0)";
    }
    mipscode.push_back(str1);

}

string get_offset(string name) //�ڵ�ǰ����������Ѱ�Ҵ�id ����offset
{
    int offset;
    int func_index = p_func.top(); //��ǰ�����±�
    int sym_index = GET_SYM_INDEX(func_index,name);
    offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;

    return to_string(offset);
}

void operand_handler(string name,string reg1) //��������� (����������,Ҫ�������ʱ�Ĵ���)
{
    string str1,offset;
    string reg = reg1;
    int len1 = name.length();

    char c1[len1]; //���ַ�������ת��Ϊ�ַ�����
    strcpy(c1, name.c_str());

    if(name == "RET") {
        reg = "$v0";
    } else if(c1[0] == '$') { //reg1
        reg = name;
        relea_occu_reg(c1[2],0); //�ͷŸú��������ʱ�Ĵ��� t2~t9
        mark_reg_type(c1[2],0); //��ʼ���üĴ�������Ϊint
    } else if(isdigit(c1[0]) || c1[0] == '+' || c1[0] == '-') { //imm1
        if(name == "0") {
            reg = "$0";
        } else {
            str1 = "li " + reg + "," + name; //addi $t0,$0,imm
            mipscode.push_back(str1);
        }
    } else if (c1[0] == '\'') { // char
        name = char_to_num(name);
        str1 = "li " + reg + "," + name; //addi $t0,$0,imm
        mipscode.push_back(str1);
    } else { //var array
        operand_id_handler(name,reg);
    }

    if (reg1 == "$t0") {
        tmp0 = reg;
    } else if(reg1 == "$t1"){
        tmp1 = reg;
    } else if(reg1 == "$v0"){ //����return���
        v0 = reg;
    } else if(reg1 == "$a0"){
        a0 = reg;
    }
}

void operand_id_handler(string name,string reg) //���� id id[exp] �Ͳ�����
{
    string str1,nature,type;
    int flag = get_array_name(name); //array judge
    if(flag) {
        name = array_name;
    }

    int func_index = p_func.top(); //��ǰ�����±�
    int sym_index = GET_SYM_INDEX(func_index,name); //���Ҿֲ�

    if(sym_index == -1) { //global
        sym_index = GET_GLOBAL_SYM_INDEX(name); //�õ�ָ����ȫ��id
        if(sym_index != -1) type = GLOBAL_TABLE[sym_index].second.type;
        if(flag) { //array
            store_arr_index(); //���������Ԫ���±��ֵ������Ĵ���$t1
            //lw $tx,label($t1)
            str1 = "lw " + reg + "," + name + "($t1)";

        } else { //var
            //lw $tx,label($0)
            str1 = "lw " + reg + "," + name + "($0)";
        }
        mipscode.push_back(str1);
    } else { //local
        int offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;
        type = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type;
        if(flag) { //array
                store_arr_index(); //���������Ԫ���±�*4��ֵ������Ĵ���$t1
                //addi $t1,$t1,offset
                string str2 = "addi $t1,$t1," + to_string(offset);
                mipscode.push_back(str2);

                string str4 = "add $s0,$s0,$t1";
                mipscode.push_back(str4);

                if(reg == "$t1") {
                    string str6 = "move $t0,$t1";
                    mipscode.push_back(str6);
                }

                //lw $tx,($s0)
                str1 = "lw " + reg + "," + "($s0)";
                mipscode.push_back(str1);

                string str5;
                if(reg == "$t1") {
                    str5 = "sub $s0,$s0,$t0";
                } else {
                    str5 = "sub $s0,$s0,$t1";
                }
                mipscode.push_back(str5);
        } else { //var
                //lw $tx,offset($s0)
                str1 = "lw " + reg + "," + to_string(offset) +"($s0)";
                mipscode.push_back(str1);
        }
    }
    if (type == type_char) {
        flag_reg_type = 1;
    } else flag_reg_type = 0;
}


void branch_handler() //cmp id1 op id2
{
    string str1,label;
    string name1 = intercode[i_inter][1];
    string name2 = intercode[i_inter][3];
    string op = intercode[i_inter][2];

    operand_handler(name1,"$t0"); //��������� ����$t0 //$tx id num (array -> $tx)
    operand_handler(name2,"$t1"); //��������� ����$t1

    i_inter++;
    if(intercode[i_inter][0] == "BZ") { //����������ת
        op = reverse_op(op);
    }
    label = intercode[i_inter][1];

    if(op == "==") { // beq x,y,label
        str1 = "beq " + tmp0 + "," + tmp1 + "," + label;
    } else if(op == "!="){ // bne x,y,label
        str1 = "bne " + tmp0 + "," + tmp1 + "," + label;
    } else if(op == "<"){ // blt x,y,label
        str1 = "blt " + tmp0 + "," + tmp1 + "," + label;
    } else if(op == "<="){ // ble x,y,label
        str1 = "ble " + tmp0 + "," + tmp1 + "," + label;
    } else if(op == ">"){ // bgt x,y,label
        str1 = "bgt " + tmp0 + "," + tmp1 + "," + label;
    } else if(op == ">="){ // bge x,y,label
        str1 = "bge " + tmp0 + "," + tmp1 + "," + label;
    }

    mipscode.push_back(str1);
}

string reverse_op(string op)
{
    string re_op;

    if(op == "==") {
        re_op = "!=";
    } else if(op == "!="){
        re_op = "==";
    } else if(op == "<"){
        re_op = ">=";
    } else if(op == "<="){
        re_op = ">";
    } else if(op == ">"){
        re_op = "<=";
    } else if(op == ">="){
        re_op = "<";
    }

    return re_op;
}

void goto_handler() // j labelX
{
    string str1 = "j " + intercode[i_inter][1];
    mipscode.push_back(str1);
}

void call_handler()
{
    int func_index = p_func.top(); //��ǰ�����±�
    int table_cnt = FUNC_MAP[func_index].second.LOCAL_TABLE.size();
    string called_func_name = intercode[i_inter][1];
    int called_func_index = GET_FUNC_INDEX(called_func_name); //�����ú����±�

    int j = 0;
    int offset;
    string str1;

    while(j < table_cnt) {
        offset = FUNC_MAP[func_index].second.LOCAL_TABLE[j].second.offset;
        str1 = "lw $t0," + to_string(offset) + "($s0)";
        mipscode.push_back(str1);
        mipscode.push_back("addi $sp,$sp,-4");
        //sw $tx,0($sp)
        str1 = "sw $t0,0($sp)";
        mipscode.push_back(str1);
        j++;
    }

    if(!p_para.empty()) { //�ñ����ú�����Ҫ���д���
        store_para_to_memory(); //�Ѵ���ֵ�����βζ�Ӧ�ڴ�
    }

    jal_handler(called_func_name); //��jal���ǰ����ж���ʱ�Ĵ����Ĵ�ȡ ��jal

    j = table_cnt - 1;

    while(j >= 0){ //��ջ ȡ��
        offset = FUNC_MAP[func_index].second.LOCAL_TABLE[j].second.offset;
        str1 = "lw $t0,0($sp)";
        mipscode.push_back(str1);
        mipscode.push_back("addi $sp,$sp,4");
        str1 = "sw $t0," + to_string(offset) + "($s0)";
        mipscode.push_back(str1);
        j--;
    }

    string called_func_type = FUNC_MAP[called_func_index].second.func_type;
    if(called_func_type == type_char) {
        flag_reg_type = 1;
    } else {
        flag_reg_type = 0;
    }

}

void call_handlershit() //��raֵ�����ڴ�,�����β��ڴ� + jal func
{
    int func_index = p_func.top(); //��ǰ�����±�
    int para_cnt = FUNC_MAP[func_index].second.para_cnt;
    string called_func_name = intercode[i_inter][1];
    int called_func_index = GET_FUNC_INDEX(called_func_name); //�����ú����±�

    int j = 0;
    int para_offset;
    string str1;

    while(j < para_cnt){ //��ջ �����β�
        para_offset = FUNC_MAP[func_index].second.LOCAL_TABLE[j].second.offset; //�β���local table�е�����
        str1 = "lw $t0," + to_string(para_offset) + "($s0)";
        mipscode.push_back(str1);
        mipscode.push_back("addi $sp,$sp,-4");
        //sw $tx,0($sp)
        str1 = "sw $t0,0($sp)";
        mipscode.push_back(str1);
        j++;
    }

    if(!p_para.empty()) { //�ñ����ú�����Ҫ���д���
        store_para_to_memory(); //�Ѵ���ֵ�����βζ�Ӧ�ڴ�
    }

    jal_handler(called_func_name); //��jal���ǰ����ж���ʱ�Ĵ����Ĵ�ȡ ��jal

    j=para_cnt-1;
    while(j >= 0){ //��ջ ȡ���β�
        para_offset = FUNC_MAP[func_index].second.LOCAL_TABLE[j].second.offset; //�β���local table�е�����
        str1 = "lw $t0,0($sp)";
        mipscode.push_back(str1);
        mipscode.push_back("addi $sp,$sp,4");
        str1 = "sw $t0," + to_string(para_offset) + "($s0)";
        mipscode.push_back(str1);
        j--;
    }

    string called_func_type = FUNC_MAP[called_func_index].second.func_type;
    if(called_func_type == type_char) {
        flag_reg_type = 1;
    } else {
        flag_reg_type = 0;
    }

}

void jal_handler(string called_func_name)
{
    string str1,reg_name;
    int func_index = p_func.top(); //��ǰ�����±�
    int i = 2; //��$t2��ʼ
    while(i < 10) {
        if(FUNC_MAP[func_index].second.reg_if_used[i] == 1) { //�üĴ�����ռ��
            reg_name = "$t" + to_string(i);
            mipscode.push_back("addi $sp,$sp,-4");
            //sw $tx,0($sp)
            str1 = "sw " + reg_name + ",0($sp)";
            mipscode.push_back(str1);
        }
        i++;
    }

    string str2 = "jal " + called_func_name;
    mipscode.push_back(str2);

    i = 9; //��$t9��ʼ
    while(i >= 2) {
        if(FUNC_MAP[func_index].second.reg_if_used[i] == 1) { //�üĴ�����ռ��
            reg_name = "$t" + to_string(i);
            str1 = "lw " + reg_name + ",0($sp)";
            mipscode.push_back(str1);
            mipscode.push_back("addi $sp,$sp,4");
        }
        i--;
    }
}

void store_para_to_memory() //�Ѵ���ֵ�����βζ�Ӧ�ڴ�
{
    string para,str1;
    string called_func_name = intercode[i_inter][1];
    int offset;
    int called_func_index = GET_FUNC_INDEX(called_func_name); //�����ú����±�
    int para_cnt = FUNC_MAP[called_func_index].second.para_cnt;
    int j = para_cnt - 1;

    while(j >= 0) { //��������ջ

        para = p_para.top();
        p_para.pop();
        operand_handler(para,"$a0"); //��Ҫ���ݵĲ�������$a0 �����para��$tx,����$a0 = $tx / $v0!
        mipscode.push_back("addi $sp,$sp,-4");
        str1 = "sw " + a0 + ",0($sp)";
        mipscode.push_back(str1);
        j--;
    }
    j = 0;
    while(j < para_cnt){ //��ջ ȡ�� �ٴ� �� �� �� �����ڴ� ����Ϊ����������ջ
        offset = FUNC_MAP[called_func_index].second.LOCAL_TABLE[j].second.offset; //�β���local table�е�����
        str1 = "lw $a0,0($sp)";
        mipscode.push_back(str1);
        mipscode.push_back("addi $sp,$sp,4");
        str1 = "sw $a0," + to_string(offset) + "($s0)";
        mipscode.push_back(str1);
        j++;
    }
}

void return_handler() //$tx (����array) /  id  /  num  /  ��a��
{
    string name = intercode[i_inter][1];
    if(name != "none") {
        operand_handler(name,"$v0"); //�Ѻ�������ֵ����$v0
        if(v0 != "$v0") {
            //move,$v0,$tx
            string str1 = "move,$v0," + v0;
            mipscode.push_back(str1);
        }
    }
    int func_index = p_func.top(); //��ǰ�����±�
    if(FUNC_MAP[func_index].first != "main") {
        mipscode.push_back("lw $ra,0($sp)");
        mipscode.push_back("addi $sp,$sp,4");
        mipscode.push_back("jr $ra");
    }
}

string char_to_num(string v) //���ַ���'a'�е��ַ���ȡ����ת��Ϊ�����ַ���
{
    int len = v.length();
    char c[len]; //���ַ�������ת��Ϊ�ַ�����
    strcpy(c, v.c_str());
    int tmp = c[1]; //�ַ�ת����
    string num = to_string(tmp);
    return num;
}
int get_array_name(string id)
{
    int re;
    int i = 0;
    int j = 0;
    int len = id.length();
    char c[len],tmp[len]; //���ַ�������ת��Ϊ�ַ�����
    char t_index[len];

    strcpy(c, id.c_str());

    for(i=0;i<len;i++) {
        if(c[i] == '[') {
            tmp[i] = '\0';
            break;
        } else {
            tmp[i] = c[i];
        }
    }
    array_name = tmp;
    re = i;

     if(c[i] == '[') {
        for(i = i+1;i<len;i++) {
            if(c[i] == ']') {
                t_index[j] = '\0';
                break;
            } else {
                t_index[j] = c[i];
                j++;
            }
        }
        array_index = t_index;
    }

    if (re == len) {
        return 0;
    } else return 1;
}

int get_array_nameshit(string id) //�ж��Ƿ�Ϊ�����ҵõ���������
{
    int re;
    int i = 0;
    int j = 0;
    int len = id.length();
    char c[len],tmp[len]; //���ַ�������ת��Ϊ�ַ�����
    char t_index[len];

    strcpy(c, id.c_str());
cout<<"sss"<<endl;
    for(i=0;i<len;i++) {
        if(c[i] == '[') {
            break;
        } else {
            tmp[i] = c[i];
        }
    }
    array_name = tmp;
    /*while(i < len) {
        if(c[i] != '[') tmp[i] = c[i];
        i++;
    }*/
    re = i;
re = 0;
    if(c[i] == '[') {
        for(i = i+1;i<len;i++) {
            if(c[i] == ']') {
                break;
            } else {
                t_index[j] = c[i];
                j++;
            }
        }
        array_index = t_index;
    }
//cout<<c[i]<<endl;
    /*if(c[i] == '[') {
        i++;
        while( i < len) {

                if(c[i] != ']') {
                   t_index[j] = c[i];
                    j++;
                }

            i++;
        }
    }*/


    //array_index = t_index;

    if (re == len) {
        return 0;
    } else return 1;
}

void store_arr_index() //���������Ԫ���±��ֵ������Ĵ���$t1
{
    //$tx +-num 'a' var const (array����ڼĴ�����)
    int offset;
    int len = array_index.length();
    char c[len]; //���ַ�������ת��Ϊ�ַ�����
    string str1;

    strcpy(c, array_index.c_str());

    if (c[0] == '$') {
        //move $t1,$tx
        str1 = "move $t1," + array_index;
        relea_occu_reg(c[2],0); //�ͷŸú��������ʱ�Ĵ��� t2~t9
    } else if (isdigit(c[0]) || c[0] == '+') {
        //li $t1,array_index
        str1 = "li $t1," + array_index;
    } else if (c[0] == '\'') {
        array_index = char_to_num(array_index);
        //li $t1,array_index
        str1 = "li $t1," + array_index;
    } else { //var const para (no array)
        int func_index = p_func.top(); //��ǰ�����±�
        int sym_index = GET_SYM_INDEX(func_index,array_index); //���Ҿֲ�

        if(sym_index == -1) { // global
            str1 = "lw $t1," + array_index + "($0)"; //lw $t0,label($0)
        } else { //local
            offset = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.offset;
            str1 = "lw $t1," + to_string(offset) + "($s0)"; //lw $t0,offset($s0)
        }
    }

    mipscode.push_back(str1);
    mipscode.push_back("sll $t1,$t1,2");
}

void relea_occu_reg(char num,int mark) //�ͷ�/ռ�� ÿ�����������ʱ�Ĵ��� t2~t9
{
    int func_index = p_func.top(); //��ǰ�����±�
    int reg_index = num - 48;
    FUNC_MAP[func_index].second.reg_if_used[reg_index] = mark; //�üĴ����ڸú����б�ռ��
}

void mark_reg_type(char num,int mark) //���üĴ������� 0 int 1 char
{
    int reg_index = num - 48;
    reg_type[reg_index] = mark;
}

void set_MEM_type(string name) //����MEM����(MEMĬ������Ϊint)
{
    int func_index = p_func.top(); //��ǰ�����±�
    int sym_index = GET_SYM_INDEX(func_index,name);
    if(sym_index != -1) FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type = "char"; //�޸�����Ϊchar
}
