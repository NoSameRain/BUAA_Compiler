#include "header.h"
#include "interCode.h"
#include "mapStore.h"
#include "errorHandler.h"

static int reg_count = 2;
static int lab_count = 0;
static int mem_count = 0;
static int str_count = 0;
static string id,type; //保存当前识别到的标识符的名字、类型
static int reg_used[10] = {1,1,0,0,0,0,0,0,0,0}; //0 unused 1 used
map<string,string> str_to_print; //需要在data段存储的字符串
static stack <string> para_to_release; //不能在push $tx后release $tx，要等所有参数被push完后再统一释放
map<string, int> char_map; //存储为char类型的变量 寄存器 MEM
int RETURN_FUNC_FLAG = 0; //当前函数为有返回值函数
int NORETURN_FUNC_FLAG = 0; //当前函数为无返回值函数
int HAS_RETURN_STA_FLAG = 0; //当前函数有返回语句
int LACK_PARENT;

static string getMemName()
{
    string num = to_string(mem_count++);
    string name = "MEM" + num;
    return name;
}
static string getRegName()
{
    string name;
    int flag = 0;
    //int flag = 1;
    int i = reg_count;

    while (reg_used[reg_count] == 1) {
        reg_count = (reg_count + 1)%10;
        if(i == reg_count) {
            flag = 1;
             //cout<<"ssss"<<endl;
            break;
        }
    }

    if(flag) { //返回变量名字 对应内存
        name = getMemName();
        STORE_VARIABLES(name,"int"); //MEM默认类型为int
    } else {
        reg_used[reg_count] = 1;
        string num = to_string(reg_count);
        name = "$t" + num;
    }
    return name;
}

static string getLabName()
{
    string name;
    lab_count = lab_count + 1;

     string num = to_string(lab_count);
     name = "LABEL" + num;

    return name;
}

static string getArrayEleName(string array_name,string index_name)
{
    string name = array_name + "[" + index_name + "]";
    return name;
}

static string getCharName(string c)
{
    string name = "\'" + c + "\'";
    return name;
}

static void release_reg(string name) //释放此寄存器
{
    int len = name.length();
    char c[len]; //将字符串变量转换为字符数组
    strcpy(c, name.c_str());

    if(c[0] == '$') {
        int num = c[2] - 48;
        reg_used[num] = 0;
        char_map.erase(name); //删除这个原本为char的寄存器
    }
}

static void release_para_table_reg() //释放值参数表里用到的寄存器
{
    int t = para_to_release.size();
    for(int i = 0;i < t;i++) {
        release_reg(para_to_release.top());
        para_to_release.pop();
    }
}

static string getStrName()
{
    str_count++;
    string num = to_string(str_count);
    string name = "STR" + num;
    return name;
}

static void storeFunc() //存储函数信息
{
    funcInfo info;
    info.func_type = type;
    STORE_FUNC(id,info);
}

static void push_GOTO_intercode(string name)
{
    vector <string> tmp;
    tmp.push_back("GOTO"); //无条件跳转至条件语句
    tmp.push_back(name);
    push_inter(tmp); //存入中间变量vec
}

static void push_label_intercode(string name)
{
    vector <string> tmp;
    tmp.push_back(name);
    tmp.push_back(":");
    push_inter(tmp); //存入中间变量vec
}

int if_is_char(string name) //$tx MEM id num 'a' 如果在map里说明为char
{
    map<string, int>::iterator it = char_map.find(name);
    if (it != char_map.end()) return 1; //在char map里找到 说明类型为char
    else return 0;
}

static void store_char(string reg,string name,int flag) //$tx MEM id 'a'
{
    int len = name.length();
    char c[len]; //将字符串变量转换为字符数组
    strcpy(c, name.c_str());

    if(c[0] == '\'') {
        char_map[name] = 1;
    } else {
        string type;
        int func_index = GET_FUNC_INDEX(FUNC_MAP.back().first);
        int sym_index = GET_SYM_INDEX(func_index,name); //先找局部

        if(flag == 0) { //find var
            if(sym_index == -1) { //global
                sym_index = GET_GLOBAL_SYM_INDEX(name); //得到指定的全局id
                if(sym_index != -1) type = GLOBAL_TABLE[sym_index].second.type;
            } else { //local
                type = FUNC_MAP[func_index].second.LOCAL_TABLE[sym_index].second.type;
            }
            if(reg == "null") {
                reg = name;
            }
        } else { //find func
            int func_index2 = GET_FUNC_INDEX(name);
            type = FUNC_MAP[func_index2].second.func_type;
        }

        if(type == "char") {
            char_map[reg] = 1;
        }
    }
}

void jSyntax() //＜程序＞::=［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
{
    int n = 0;

    getSym();

    if (cateCode[cateInd] == "CONSTTK") {
        constantSpeci(); //常量说明(getsym)
    }
    if (cateCode[cateInd] == "INTTK" ||
        cateCode[cateInd] == "CHARTK") {
        jLexical(); //预读
        n = n + re_Track.back();
        jLexical();
        n = n + re_Track.back();
        ind = ind - n;
        if (cateCode[cate] == "COMMA" || cateCode[cate] == "SEMICN" || cateCode[cate] == "LBRACK"){
            variableSpeci(); //变量说明 get
        }
    }


    while(cateCode[cateInd] == "INTTK" ||
          cateCode[cateInd] == "CHARTK" ||
          cateCode[cateInd] == "VOIDTK") {
        char_map.clear(); //initial
        if (cateCode[cateInd] == "INTTK" ||
            cateCode[cateInd] == "CHARTK") {
            ReturnFunc(); //有返回函数
        }else if(cateCode[cateInd] == "VOIDTK") {
            getSym();
            if (cateCode[cateInd] == "IDENFR"){
                noReturnFunc(); //无返回函数
            } else if(cateCode[cateInd] == "MAINTK"){
                mainFunc(); //主函数
                break;
            }
        }
        getSym();
    }
}

void constantSpeci() //常量说明::=const＜常量定义＞;{ const＜常量定义＞;} //getsym
{

    while (cateCode[cateInd] == "CONSTTK") {
        getSym();
        constantDefin(); //常量定义+getsym
        if (cateCode[cateInd] != "SEMICN") {
            error("constantSpeci");
            K_SHOULD_BE_SEMICN(); //应该为分号
        } else getSym();
    }
}

void variableSpeci() //变量说明::=?＜变量定义＞;{＜变量定义＞;} //getsym
{
    int n = 0;
    while (cateCode[cateInd] == "INTTK" ||
           cateCode[cateInd] == "CHARTK") {
        jLexical(); //预读
        n = n + re_Track.back();
        jLexical();
        n = n + re_Track.back();
        ind = ind - n;
        n = 0;
        if (cateCode[cate] != "COMMA" && cateCode[cate] != "SEMICN" && cateCode[cate] != "LBRACK") break;
        variableDefin();
        if (cateCode[cateInd] != "SEMICN") {

            K_SHOULD_BE_SEMICN(); //应该为分号
            error("variableSpeci");
        } else getSym();
    }
}

void ReturnFunc() //有返回函数::= ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}' (复合语句能为空吗return?）
{
    RETURN_FUNC_FLAG = 1;

    vector <string> tmp;
    tmp.push_back("func"); // func
    tmp.push_back(s_word); // type_inter

    type = s_word;

    declaHeader();

    id = s_word;
    storeFunc(); //存储函数信息

    tmp.push_back(s_word); // id
    push_inter(tmp); //存入中间变量vec
    getSym();
    if(cateCode[cateInd] != "LPARENT") error("ReturnFunc1"); //(

    getSym();
    if (cateCode[cateInd] == "INTTK" ||
        cateCode[cateInd] == "CHARTK") { //参数表不为空
        paraTable(); //参数表 + getsym
        if(cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("ReturnFunc3");
        } else getSym();
    }else if(cateCode[cateInd] == "RPARENT"){ //) //参数表为空
        getSym();
    }else {
        L_LACK_RPARENT(); //缺少右小括号
        error("ReturnFunc2");
    }

    if(cateCode[cateInd] != "LBRACE") error("ReturnFunc4"); //{
    getSym();
    HAS_RETURN_STA_FLAG = 0;

    compoundStatemnet(); //复合语句 + getsym

    if(cateCode[cateInd] != "RBRACE") error("ReturnFunc5"); //}
    if(HAS_RETURN_STA_FLAG == 0) H_Return_Lack(); //有返回值函数缺少返回语句


    RETURN_FUNC_FLAG = 0;
    HAS_RETURN_STA_FLAG = 0;
}

void noReturnFunc() //无返回函数::=?void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
{
    id = s_word;
    type = "void";
    NORETURN_FUNC_FLAG = 1; //当前函数为无返回值函数

    storeFunc(); //存储函数信息

    vector <string> tmp;
    tmp.push_back("func"); // func
    tmp.push_back("void"); // type_inter
    tmp.push_back(id); // id
    push_inter(tmp); //存入中间变量vec

    getSym();
    if(cateCode[cateInd] != "LPARENT") error("noReturnFunc1"); //(

    getSym();
    if (cateCode[cateInd] == "INTTK" ||
        cateCode[cateInd] == "CHARTK") { //参数表不为空
        paraTable(); //参数表 + getsym
        if(cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("noReturnFunc2");
        } else getSym();
    }else if(cateCode[cateInd] == "RPARENT"){ //) //参数表为空
        getSym();
    }else {
        L_LACK_RPARENT(); //缺少右小括号
        error("noReturnFunc2");
    }

    if(cateCode[cateInd] != "LBRACE") error("noReturnFunc"); //{
    getSym();
    compoundStatemnet(); //复合语句 + getsym
    if(cateCode[cateInd] != "RBRACE") error("noReturnFunc"); //}

    NORETURN_FUNC_FLAG = 0; //当前函数为无返回值函数
}

void mainFunc() //主函数::= void main‘(’‘)’ ‘{’＜复合语句＞‘}'
{
    id = "main";
    type = "void";
    NORETURN_FUNC_FLAG = 1; //当前函数为无返回值函数

    storeFunc(); //存储函数信息

    vector <string> tmp;
    tmp.push_back("func"); // func
    tmp.push_back("void"); // type_inter
    tmp.push_back(s_word); // id
    push_inter(tmp); //存入中间变量vec

    getSym();
    if(cateCode[cateInd] != "LPARENT") error("mainFunc"); //(
    getSym();
    if(cateCode[cateInd] != "RPARENT") {
        L_LACK_RPARENT(); //缺少右小括号
        error("mainFunc"); //)
    } else getSym();
    if(cateCode[cateInd] != "LBRACE") error("mainFunc"); //{
    getSym();
    compoundStatemnet(); //复合语句 + getsym
    if(cateCode[cateInd] != "RBRACE") error("mainFunc"); //}

    NORETURN_FUNC_FLAG = 0; //当前函数为无返回值函数
}

//常量定义::= ? int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
//| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
void constantDefin() //常量定义 //getsym
{
    int flag = 1;
    string value;
    vector <string> tmp;

    tmp.push_back("const");

    if (cateCode[cateInd] != "INTTK" &&
        cateCode[cateInd] != "CHARTK") error("constantDefin");
    if (cateCode[cateInd] == "CHARTK") flag = 0;
    type = s_word;
    tmp.push_back(type); //type

    getSym();
    if (cateCode[cateInd] != "IDENFR") error("constantDefin");
    id = s_word;
    tmp.push_back(id); //id

    getSym();
    if (cateCode[cateInd] != "ASSIGN") error("constantDefin"); //=
    tmp.push_back(s_word);//=

    getSym();


    if(flag) {
        value = jInteger(); //整数
        if(value == "null") O_constDefTypeError(); //常量定义中=后面只能是整型或字符型常量
    }  else{
        if (cateCode[cateInd] != "CHARCON") {
            error("constantDefin"); //字符
            O_constDefTypeError(); //常量定义中=后面只能是整型或字符型常量
        }
        value = getCharName(s_word);
    }

    tmp.push_back(value);//value
    push_inter(tmp); //存入中间变量vec

    STORE_CONSTANTS(id,type,value); //储存常量

    getSym();
    while(cateCode[cateInd] == "COMMA"){
        vector <string> tmp1;
        tmp1.push_back("const");
        tmp1.push_back(type); //type_inter
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("constantDefin");
        tmp1.push_back(s_word); //id
        id = s_word;
        getSym();
        if (cateCode[cateInd] != "ASSIGN") error("constantDefin"); //=
        tmp1.push_back(s_word);//=
        getSym();
        if(flag) {
            value = jInteger(); //整数
            if(value == "null") O_constDefTypeError(); //常量定义中=后面只能是整型或字符型常量
        } else{
            if (cateCode[cateInd] != "CHARCON") {
                error("constantDefin"); //字符
                O_constDefTypeError(); //常量定义中=后面只能是整型或字符型常量
            }
            value = getCharName(s_word);
        }
        tmp1.push_back(value);//value

        push_inter(tmp1); //存入中间变量vec
        STORE_CONSTANTS(id,type,value); //储存常量

        getSym();
    }
}

//变量定义::=?＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']')
//{,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
void variableDefin() //变量定义 //getsym
{
    vector <string> tmp;
    type = s_word;

    tmp.push_back("var");
    tmp.push_back(type);
    getSym();
    if (cateCode[cateInd] != "IDENFR") error("variableDefin"); //标志符

    id = s_word;
    getSym();

    if(cateCode[cateInd] == "LBRACK") {
        getSym();
        unsignInteger(); //无符号整数
        STORE_ARRAY(id,type,s_word);

        string ele_name = getArrayEleName(id,s_word);
        tmp.push_back(ele_name); //array element name

        getSym();
        if (cateCode[cateInd] != "RBRACK") {
            M_LACK_RBRACK(); //缺少右中括号
            error("variableDefin"); // )
        } else getSym();
    } else {
        tmp.push_back(id); // id
        STORE_VARIABLES(id,type);
    }

    push_inter(tmp); //存入中间变量vec

    while(cateCode[cateInd] == "COMMA"){
        vector <string> tmp;
        tmp.push_back("var");
        tmp.push_back(type);
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("variableDefin");
        id = s_word;
        getSym();
        if(cateCode[cateInd] == "LBRACK") {
            getSym();
            unsignInteger(); //无符号整数
            STORE_ARRAY(id,type,s_word);

            string ele_name = getArrayEleName(id,s_word);
            tmp.push_back(ele_name); //array element name

            getSym();
            if (cateCode[cateInd] != "RBRACK") {
                M_LACK_RBRACK(); //缺少右中括号
                error("variableDefin"); // )
            } else getSym();
        } else {
            tmp.push_back(id); // id
            STORE_VARIABLES(id,type);
        }
        push_inter(tmp); //存入中间变量vec
    }
}

void declaHeader() //声明头部::=int＜标识符＞|char＜标识符＞
{
    getSym();
    if (cateCode[cateInd] != "IDENFR") error("declaHeader");
}

void paraTable() //参数表::= ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}|＜空＞！！！ //getsym
{
    //类型标识符在有、无返回值类型中p预读判断过了
    vector <string> tmp;
    tmp.push_back("para");
    tmp.push_back(s_word); //type_inter
    type = s_word;

    getSym();
    if (cateCode[cateInd] != "IDENFR") error("paraTable");

    id = s_word;
    STORE_PARATABLE(id,type); // 向当前函数的作用域中存入参数信息

    tmp.push_back(s_word); //id
    push_inter(tmp); //存入中间变量vec

    getSym();

    while(cateCode[cateInd] == "COMMA"){
        vector <string> tmp1;
        tmp1.push_back("para");
        getSym();
        //type_interIdenti(); //类型标识符
        tmp1.push_back(s_word); //type_inter
        type = s_word;
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("variableDefin");
        tmp1.push_back(s_word); //id
        push_inter(tmp1); //存入中间变量vec

        id = s_word;
        STORE_PARATABLE(id,type); // 向当前函数的作用域中存入参数信息

        getSym();
    }
}

void compoundStatemnet() //复合语句::=［＜常量说明＞］［＜变量说明＞］＜语句列＞ (可能为空:只有';') //getsym
{
    //之前已经getsym了
    if (cateCode[cateInd] == "CONSTTK") {
        constantSpeci(); //常量说明(getsym)
    }
    if (cateCode[cateInd] == "INTTK" ||
        cateCode[cateInd] == "CHARTK") {
        variableSpeci(); //变量说明（getsym)
    }
    statementCol(); //语句列 + getsym
}

void statementCol() //语句列::=｛＜语句＞｝（可能只有';'）//getsym 0-duoge yuju
{
    int flag = 0;
    //cout<<s_word<<endl;
    while(cateCode[cateInd] == "IFTK" || cateCode[cateInd] == "WHILETK"||
          cateCode[cateInd] == "LBRACE" || cateCode[cateInd] == "IDENFR" ||
          cateCode[cateInd] == "SCANFTK" || cateCode[cateInd] == "PRINTFTK" ||
          cateCode[cateInd] == "RETURNTK" || cateCode[cateInd] == "DOTK" ||
          cateCode[cateInd] == "FORTK" || cateCode[cateInd] == "SEMICN"){
        jStatement(); //语句 + getsym
        flag = 1;
    }
    if(flag == 0) {
        K_SHOULD_BE_SEMICN(); //应该为分号
        error("statementCol");
    }
}
//语句 ::=＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'|＜有返回值函数调用语句＞;
// |＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜返回语句＞;
void jStatement() //语句 + getsym
{
    if (cateCode[cateInd] == "IFTK") {
        ifSta(); //条件语句 + getsym
    } else if (cateCode[cateInd] == "WHILETK" ||
               cateCode[cateInd] == "FORTK" ||
               cateCode[cateInd] == "DOTK") {
        loopSta(); //循环语句 + getsym
    } else if (cateCode[cateInd] == "LBRACE") { // {
        getSym();
        statementCol(); //语句列 + getsym
        if (cateCode[cateInd] != "RBRACE") error("jStatement1"); // }
        getSym();
    } else if (cateCode[cateInd] == "IDENFR") {
        call_Assi_Sta(s_word); //调用及赋值语句 + getsym
        if (cateCode[cateInd] != "SEMICN") {
            K_SHOULD_BE_SEMICN(); //应该为分号
            error("jStatement2"); // ;
        } else getSym();
    } else if (cateCode[cateInd] == "SCANFTK") {
        scanfSta(); //读语句
        if (cateCode[cateInd] != "SEMICN") {
            K_SHOULD_BE_SEMICN(); //应该为分号
            error("jStatement3"); // ;
        } else getSym();
    } else if (cateCode[cateInd] == "PRINTFTK") {
        PrintfSta(); //写语句
        if (cateCode[cateInd] != "SEMICN") {
            K_SHOULD_BE_SEMICN(); //应该为分号
            error("jStatement4"); // ;
        } else getSym();
    } else if (cateCode[cateInd] == "RETURNTK") {
        if(RETURN_FUNC_FLAG == 1)HAS_RETURN_STA_FLAG = 1;  //当前函数是有返回函数且有返回语句
        getSym();
        if (cateCode[cateInd] == "SEMICN") { // ;
            if(RETURN_FUNC_FLAG == 1) H_Return_Lack(); //有返回值函数缺少返回值

            vector <string> tmp;
            tmp.push_back("ret");
            tmp.push_back("none");
            push_inter(tmp); //存入中间变量vec

            getSym();
        } else if (cateCode[cateInd] == "RBRACE") { //}
            error("jStatement5"); // ;
            K_SHOULD_BE_SEMICN(); //应该为分号
        } else {
            returnSta(); //有返回值返回语句 + getsym

            if(NORETURN_FUNC_FLAG) G_Return_Surplus(); //无返回值函数return(***)错误
            if (cateCode[cateInd] == "SEMICN"){
                getSym();
            } else {
                error("jStatement6"); // ;
                K_SHOULD_BE_SEMICN(); //应该为分号
            }
        }
    } else if(cateCode[cateInd] == "SEMICN"){ // 空 ;
        getSym();
    }else error("jStatement7");
}

void ifSta() //条件语句::= if '('＜条件＞')'＜语句＞［else＜语句＞］ //getsym
{
    getSym();
    if (cateCode[cateInd] != "LPARENT") error("ifSta"); // (
    getSym();
    condition(); //条件 + getsym
    if (cateCode[cateInd] != "RPARENT") {
        L_LACK_RPARENT(); //缺少右小括号
        error("ifSta"); // )
    } else getSym();

    vector <string> tmp1;
    tmp1.push_back("BZ"); //不满足则跳转
    string lab_name1 = getLabName();
    tmp1.push_back(lab_name1);
    push_inter(tmp1); //存入中间变量vec

    jStatement(); //语句 + getsym
    if(cateCode[cateInd] == "ELSETK") { //else
        string lab_name2 = getLabName(); //else语句结束后的语句

        push_GOTO_intercode(lab_name2);
        push_label_intercode(lab_name1);

        getSym();
        jStatement(); //语句 + getsym

        push_label_intercode(lab_name2);
    } else {
        push_label_intercode(lab_name1);
    }
}

//条件::=??＜表达式＞＜关系运算符＞＜表达式＞//整型表达式之间才能进行关系运算｜＜表达式＞
//表达式为整型，其值为0条件为假，值不为0时条件为真
void condition() //条件 + getsym
{
    int flag_char2 = 1;
    vector <string> tmp;
    tmp.push_back("cmp");
    string name1 = expression(); //表达式 + getsym
    int flag_char1 = if_is_char(name1); //$tx MEM id num 'a' 如果在map里说明为char
    tmp.push_back(name1);
    if(relationOp()) { //关系运算符
        tmp.push_back(s_word); //op
        getSym();
        string name2 = expression(); //表达式 + getsym
        tmp.push_back(name2);
        flag_char2 = if_is_char(name2); //$tx MEM id num 'a' 如果在map里说明为char
        release_reg(name1); //释放此寄存器
        release_reg(name2); //释放此寄存器
    } else {
        tmp.push_back("!="); //op
        tmp.push_back("0"); //0
    }
    if(flag_char1 == 1 || flag_char2 == 1) F_IllTypeInCondition(); //条件判断中出现不合法类型
    push_inter(tmp); //存入中间变量vec
}
int relationOp() //关系运算符 "LSS","LEQ","GRE","GEQ","EQL","NEQ"
{
    if(cateCode[cateInd] == "LSS" || cateCode[cateInd] == "LEQ" ||
       cateCode[cateInd] == "GRE" || cateCode[cateInd] == "GEQ" ||
       cateCode[cateInd] == "EQL" || cateCode[cateInd] == "NEQ") {
        return 1;
    } else return 0;
}

//＜循环语句＞::= while '('＜条件＞')'＜语句＞|
//do＜语句＞while '('＜条件＞')' |
//for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
void loopSta() //循环语句 + getsym
{
    if (cateCode[cateInd] == "WHILETK") {
        string lab_name1 = getLabName();

        push_label_intercode(lab_name1);

        getSym();
        if (cateCode[cateInd] != "LPARENT") error("loopSta"); // (
        getSym();
        condition(); //条件 + getsym
        if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("loopSta"); // )
        } else getSym();

        vector <string> tmp2;
        tmp2.push_back("BZ"); //不满足则跳转
        string lab_name2 = getLabName();
        tmp2.push_back(lab_name2);
        push_inter(tmp2); //存入中间变量vec

        jStatement(); //语句 + getsym

        push_GOTO_intercode(lab_name1);

        push_label_intercode(lab_name2);
    } else if (cateCode[cateInd] == "DOTK"){
        string lab_name1 = getLabName();
        push_label_intercode(lab_name1);

        getSym();
        jStatement(); //语句 + getsym
        if (cateCode[cateInd] != "WHILETK") {
            error("loopSta");
            N_LACK_WHILE(); //do while缺少while
        } else getSym();
        if (cateCode[cateInd] != "LPARENT") error("loopSta"); // (
        getSym();
        condition(); //条件 + getsym

        vector <string> tmp2;
        tmp2.push_back("BNZ"); //满足则跳转
        tmp2.push_back(lab_name1);
        push_inter(tmp2); //存入中间变量vec

        if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("loopSta"); // )
        } else getSym();
    } else if (cateCode[cateInd] == "FORTK") {
        vector <string> tmp5;
        getSym();
        if (cateCode[cateInd] != "LPARENT") error("loopSta"); // (
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("loopSta"); // 标识符
        tmp5.push_back(s_word); // id

        C_unDefName(0,s_word); //查找变常量
        J_CHANGE_CONST_VALUE(s_word); //改变常量的值

        getSym();
        if (cateCode[cateInd] != "ASSIGN") error("loopSta"); // =
        tmp5.push_back(s_word); // =
        getSym();
        string name = expression(); //表达式 + getsym
        tmp5.push_back(name); // exp
        push_inter(tmp5); //存入中间变量vec
        release_reg(name); //释放此寄存器

        if (cateCode[cateInd] != "SEMICN") {
            K_SHOULD_BE_SEMICN(); //应该为分号
            error("loopSta"); // ;
        } else getSym();

        string lab_name1 = getLabName();
        push_label_intercode(lab_name1);

        condition(); //条件 + getsym

        vector <string> tmp2;
        tmp2.push_back("BZ"); //不满足则跳转
        string lab_name2 = getLabName();
        tmp2.push_back(lab_name2);
        push_inter(tmp2); //存入中间变量vec

        vector <string> tmp;

        if (cateCode[cateInd] != "SEMICN") {
            K_SHOULD_BE_SEMICN(); //应该为分号
            error("loopSta"); // ;
        } else getSym();
        if (cateCode[cateInd] != "IDENFR") error("loopSta"); // 标识符
        tmp.push_back(s_word); // id

        C_unDefName(0,s_word); //查找变常量
        J_CHANGE_CONST_VALUE(s_word); //改变常量的值

        getSym();
        if (cateCode[cateInd] != "ASSIGN") error("loopSta"); // =
        tmp.push_back(s_word); // =
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("loopSta"); // 标识符
        tmp.push_back(s_word); // id
        C_unDefName(0,s_word); //查找变常量
        J_CHANGE_CONST_VALUE(s_word); //改变常量的值
        getSym();
        if (cateCode[cateInd] != "PLUS" && cateCode[cateInd] != "MINU") error("loopSta"); // + / -
        tmp.push_back(s_word); // op
        getSym();
        step(); //步长
        tmp.push_back(s_word); // num
        getSym();

        if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("loopSta"); // )
        } else getSym();

        jStatement(); //语句 + getsym
        push_inter(tmp); //存入中间变量vec

        push_GOTO_intercode(lab_name1);
        push_label_intercode(lab_name2);
    }
}

void call_Assi_Sta(string gw) //调用及赋值语句 + getsym
{
    getSym();
    if (cateCode[cateInd] == "ASSIGN" || cateCode[cateInd] == "LBRACK") { // = / [
        assignSta(gw); //赋值语句 + getsym
    } else if (cateCode[cateInd] == "LPARENT" ) { // (
        //调用语句::=＜标识符＞'('＜值参数表＞')
        C_unDefName(1,gw); //找函数
        getSym();
        if (cateCode[cateInd] == "RPARENT" ) {
            vector <string> para_vec;
            D_E_valParaTableError(gw,para_vec); //函数参数个数、类型是否匹配
            getSym();
        } else {
            valParaTable(gw); //值参数表 + getsym
            release_para_table_reg(); //释放值参数表里用到的寄存器
            if (cateCode[cateInd] != "RPARENT" ) {
                error("call_Assi_Sta"); // )
                L_LACK_RPARENT(); //缺少右小括号
            } else getSym();
        }
        vector <string> tmp;
        tmp.push_back("call");
        tmp.push_back(gw); //id
        push_inter(tmp); //存入中间变量vec

    } else error("call_Assi_Sta");
}

void assignSta(string gw) //赋值语句 ::= ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞ + getsym
{
    vector <string> tmp;

    if (cateCode[cateInd] == "ASSIGN") { // =
        C_unDefName(0,gw); //查找变常量
        J_CHANGE_CONST_VALUE(gw); //改变常量的值

        tmp.push_back(gw); //id
        tmp.push_back("="); //=
        getSym();
        string name = expression(); //表达式 + getsym
        tmp.push_back(name); //id or reg
        release_reg(name); //释放此寄存器
    } else if (cateCode[cateInd] == "LBRACK") { // [
        C_unDefName(0,gw); //查找数组
        getSym();
        string index_name = expression(); //表达式 + getsym

        I_ARRAY_INDEX_ERROR(index_name); //数组下标应为整型表达式

        string ele_name = getArrayEleName(gw,index_name);
        tmp.push_back(ele_name); //array element name

        if (cateCode[cateInd] != "RBRACK") {
            M_LACK_RBRACK(); //缺少右中括号
            error("assignSta"); // )
        } else getSym();

        if (cateCode[cateInd] != "ASSIGN") error("assignSta"); // =
        tmp.push_back("="); //=
        getSym();

        string name2 = expression(); //表达式 + getsym
        tmp.push_back(name2); //id or reg
        release_reg(index_name); //释放此寄存器
        release_reg(name2); //释放此寄存器
    }
    push_inter(tmp); //存入中间变量vec
}

void scanfSta() //读语句::=scanf'('＜标识符＞{,＜标识符＞}')'
{
    vector <string> tmp;

    getSym();
    if (cateCode[cateInd] != "LPARENT") error("scanfSta"); // (

    getSym();
    if (cateCode[cateInd] != "IDENFR") error("scanfSta"); //标识符
    C_unDefName(0,s_word); //查找变常量

    tmp.push_back("scanf"); //读标志
    tmp.push_back(s_word); //id
    push_inter(tmp); //存入中间变量vec

    getSym();
    while(cateCode[cateInd] == "COMMA") {
        vector <string> tmp1;
        getSym();
        if (cateCode[cateInd] != "IDENFR") error("scanfSta");
        C_unDefName(0,s_word); //查找变常量

        tmp1.push_back("scanf"); //读标志
        tmp1.push_back(s_word); //id
        push_inter(tmp1); //存入中间变量vec
        getSym();
    }

     if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("scanfSta"); // )
    } else getSym();
}

void PrintfSta() //写语句::=printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
{
    vector <string> tmp;
    string r;

    getSym();
    if (cateCode[cateInd] != "LPARENT") error("scanfSta"); // (

    getSym();
    if (cateCode[cateInd] == "STRCON"){
        tmp.push_back("printf"); //写标志
        string name = getStrName();
        str_to_print.insert(pair<string,string>(name,s_word));
        tmp.push_back(name); //字符串标志
        tmp.push_back(s_word); //strcon

        getSym();
        if (cateCode[cateInd] == "COMMA") {
            tmp.push_back(","); //表示后面还有id
            getSym();
            r = expression(); //表达式 + getsym
            tmp.push_back(r); //exp
            release_reg(r); //释放此寄存器
        }
        push_inter(tmp); //存入中间变量vec
    } else {
        vector <string> tmp1;
        r = expression(); //表达式 + getsym
        tmp1.push_back("printf"); //写标志
        tmp1.push_back(r); //exp
        release_reg(r); //释放此寄存器
        push_inter(tmp1); //存入中间变量vec
    }

    if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("PrintfSta"); // )
    } else getSym();
}

void returnSta() //返回语句::=return['('＜表达式＞')']
{
    if (cateCode[cateInd] == "LPARENT"){
        vector <string> tmp;
        tmp.push_back("ret");
        getSym();
        string name = expression(); //表达式 + getsym

        if(RETURN_FUNC_FLAG == 1) H_Return_unMatch(name); //返回值类型不匹配

        tmp.push_back(name); // id
        release_reg(name); //释放此寄存器
        push_inter(tmp); //存入中间变量vec

        if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("returnSta"); // )
        } else getSym();
    } else{
        error("returnSta");
        printError("h");
    }
}

string expression() //表达式::=［＋｜－］＜项＞{＜加法运算符＞＜项＞} //[+|-]只作用于第一个<项> //getsym
{
    string rename,name,name2,t1;
    int flag = 0;

    if(cateCode[cateInd] == "PLUS" || cateCode[cateInd] == "MINU") {
        if(cateCode[cateInd] == "MINU") flag = 1;
        getSym();
    }
    name2 = term(); //项 + getsym

    if(flag) { //-只作用于第一个<项>
        vector <string> tmp;
        name = getRegName();//reg
        tmp.push_back(name); // reg
        tmp.push_back("=");
        tmp.push_back("0");
        tmp.push_back("-"); //+ -
        tmp.push_back(name2); // reg
        release_reg(name2); //释放此寄存器
        push_inter(tmp); //存入中间变量vec
        name2 = name; //这里非常玄!
    }

    if(cateCode[cateInd] != "PLUS" && cateCode[cateInd] != "MINU") {
        rename = name2; //term
    }

    while(cateCode[cateInd] == "PLUS" || cateCode[cateInd] == "MINU") {
        vector <string> tmp;
        name = getRegName();//reg
        rename = name;
        tmp.push_back(name); // reg
        tmp.push_back("=");
        tmp.push_back(name2); //term : reg or id

        tmp.push_back(s_word); //+ -
        getSym();
        t1 = term(); //项 + getsym
        tmp.push_back(t1); //id
        release_reg(name2); //释放此寄存器 进入递归前不要释放
        release_reg(t1); //释放此寄存器
        push_inter(tmp); //存入中间变量vec
        name2 = name;
    }

    return rename;
}

string term() //项::=＜因子＞{＜乘法运算符＞＜因子＞} //getsym
{
    string rename,name,name2,t1;

    name2 = factor(); //因子  getsym

    if(cateCode[cateInd] != "MULT" && cateCode[cateInd] != "DIV") {
        rename = name2; //id
    }

    while(cateCode[cateInd] == "MULT" || cateCode[cateInd] == "DIV") {
        vector <string> tmp;
        name = getRegName();//reg
        rename = name;
        tmp.push_back(name); // reg
        tmp.push_back("=");
        tmp.push_back(name2); //term : reg or id
        tmp.push_back(s_word); //* /
        getSym();

        t1 = factor(); //因子  getsym

        tmp.push_back(t1); //id
        release_reg(name2); //释放此寄存器
        release_reg(t1); //释放此寄存器
        push_inter(tmp); //存入中间变量vec
        name2 = name;
    }

    return rename;
}
//因子:=＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜
//＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
string factor() //因子 + getsym
{
    string rename;
    vector <string> tmp;

    if (cateCode[cateInd] == "IDENFR") {
        string gw = s_word; //1
        getSym();
        if (cateCode[cateInd] == "LBRACK") { //[
            string reg_name = getRegName();//reg
            rename = reg_name;
            tmp.push_back(reg_name); // reg
            tmp.push_back("=");
            getSym();
            string index_name = expression(); //表达式 + getsym
            I_ARRAY_INDEX_ERROR(index_name); //整型表达式
            string ele_name = getArrayEleName(gw,index_name);
            tmp.push_back(ele_name);
            release_reg(index_name); //释放此寄存器
            push_inter(tmp); //存入中间变量vec

            if (cateCode[cateInd] != "RBRACK") {
                M_LACK_RBRACK(); //缺少右中括号
                error("assignSta"); // )
            } else getSym();

            store_char(reg_name,gw,0); //找变量
        } else if (cateCode[cateInd] == "LPARENT" ) { //(
            //调用语句::=＜标识符＞'('＜值参数表＞')'
            C_unDefName(1,gw);

            getSym();

            if (cateCode[cateInd] == "RPARENT" ) {
                vector <string> para_vec;
                D_E_valParaTableError(gw,para_vec); //函数参数个数、类型是否匹配
                getSym();
            } else if (cateCode[cateInd] == "MULT"
                       || cateCode[cateInd] == "DIV"|| cateCode[cateInd] == "LSS"|| cateCode[cateInd] == "LEQ"
                       || cateCode[cateInd] == "GRE" || cateCode[cateInd] == "GEQ" || cateCode[cateInd] == "EQL"
                       || cateCode[cateInd] == "NEQ" || cateCode[cateInd] == "ASSIGN" || cateCode[cateInd] == "SEMICN"
                       || cateCode[cateInd] == "COMMA" || cateCode[cateInd] == "RBRACK" || cateCode[cateInd] == "RBRACE") {
                L_LACK_RPARENT(); //缺少右小括号
                error("factor"); // )
            }
            else if (cateCode[cateInd] != "RPARENT" ) {
                valParaTable(gw); //值参数表 + getsym
                release_para_table_reg(); //释放值参数表里用到的寄存器
                if (cateCode[cateInd] != "RPARENT") {
                    L_LACK_RPARENT(); //缺少右小括号
                    error("factor"); // )
                }else getSym();
            }
            tmp.push_back("call");
            tmp.push_back(gw); //id
            push_inter(tmp); //存入中间变量vec

            vector <string> tmp2;
            string reg_name = getRegName();//reg
            rename = reg_name;
            tmp2.push_back(reg_name); // reg
            tmp2.push_back("=");
            tmp2.push_back("RET");
            push_inter(tmp2); //存入中间变量vec

            store_char(reg_name,gw,1); //找函数
        } else {
            C_unDefName(0,gw);
            store_char("null",gw,0);
            rename = gw;
        }
    } else if (cateCode[cateInd] == "LPARENT") {
        getSym();
        string name = expression(); //表达式 + getsym

        if (cateCode[cateInd] != "RPARENT") {
            L_LACK_RPARENT(); //缺少右小括号
            error("factor"); // )
        } else getSym();

        //避免print分不清('a') \ 'a'
        vector <string> tmp2;
        string reg_name = getRegName();//reg
        rename = reg_name;
        tmp2.push_back(reg_name); // reg
        tmp2.push_back("=");
        tmp2.push_back("0");
        tmp2.push_back("+");
        tmp2.push_back(name);
        push_inter(tmp2); //存入中间变量vec
        release_reg(name); //释放此寄存器
    } else if(cateCode[cateInd] == "PLUS" || cateCode[cateInd] == "MINU" || cateCode[cateInd] == "INTCON") {//
        rename = jInteger();
        getSym();
    } else if(cateCode[cateInd] == "CHARCON") {
        rename = getCharName(s_word);
        getSym();
        store_char("null",rename,0);
    } else error("factor");

    return rename;
}

void step() //步长::=＜无符号整数＞
{
    if (unsignInteger()) {
    } else error("step");

}

void valParaTable(string gw) //值参数表:=＜表达式＞{,＜表达式＞}｜＜空＞!! + getsym
{
    vector <string> tmp1;
    vector <string> para_vec;
    string name1 = expression(); //表达式 + getsym
    para_vec.push_back(name1);
    tmp1.push_back("push");
    tmp1.push_back(name1);
    push_inter(tmp1); //存入中间变量vec
    para_to_release.push(name1);//release_reg(name1)
    while(cateCode[cateInd] == "COMMA") {
        vector <string> tmp2;
        getSym();
        string name2 = expression(); //表达式 + getsym
        para_vec.push_back(name2);
        tmp2.push_back("push");
        tmp2.push_back(name2);
        push_inter(tmp2); //存入中间变量vec
        para_to_release.push(name2);//release_reg(name1)
    }
    D_E_valParaTableError(gw,para_vec); //函数参数个数、类型是否匹配
}

string jInteger() //整数:=［＋｜－］＜无符号整数
{
    string rename = "null";
    if (cateCode[cateInd] == "PLUS" || cateCode[cateInd] == "MINU"){
        string op = s_word;
        getSym();
        if(unsignInteger()){ //无符号整数
            rename = op + s_word;
        }
    } else {
        if(unsignInteger()){ //无符号整数
            rename = s_word;
        }
    }
    return rename;
}

int unsignInteger() //无符号整数:=＜非零数字＞｛＜数字＞｝| 0
{
    if (cateCode[cateInd] == "INTCON") {
        return 1;
    }else{
        error("unsignInteger");
        return 0;
    }
}

void type_interIdenti() //类型标识符
{
    if (cateCode[cateInd] != "INTTK" &&
        cateCode[cateInd] != "CHARTK") error("paraTable");
}



