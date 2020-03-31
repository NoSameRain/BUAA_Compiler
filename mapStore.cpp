#include "header.h"
#include "mapStore.h"
#include "errorHandler.h"

vector<pair<string, symbolTable>> GLOBAL_TABLE; //全局常变量表 id + info
vector<pair<string, funcInfo>> FUNC_MAP; // 函数表(id + info) //FUNC_MAP.back().

int FIND_IN_GLOBAL(string id) //在全局变 常量中找
{
    int f1 = 0;

    if(!GLOBAL_TABLE.empty()) {
        for (int i=0;i<GLOBAL_TABLE.size();i++) {
            if(GLOBAL_TABLE[i].first == id) {
                f1 = 1;
                break;
            }
        }
    }

    return f1;
}

int FIND_IN_LOCAL(string id) //在当前函数作用域的参数变常量里找
{
    int flag = -1;
    for (int i=0;i<FUNC_MAP.back().second.LOCAL_TABLE.size();i++) {
        if(FUNC_MAP.back().second.LOCAL_TABLE[i].first == id) {
            flag = i;
            break;
        }
    }

    return flag;
}

int GET_FUNC_INDEX(string func_name)
{
    int func_index = -1;
    for (int i=0;i<FUNC_MAP.size();i++) {
        if(FUNC_MAP[i].first == func_name) {
            func_index = i;
            break;
        }
    }
    return func_index;
}

int GET_SYM_INDEX(int func_index,string sym_name) //得到指定函数中的指定id 没找到返回-1
{
    int sym_index = -1;
    for (int i=0;i<FUNC_MAP[func_index].second.LOCAL_TABLE.size();i++) {
        if(FUNC_MAP[func_index].second.LOCAL_TABLE[i].first == sym_name) {
            sym_index = i;
            break;
        }
    }

    return sym_index;
}

int GET_GLOBAL_SYM_INDEX(string sym_name) //得到指定的全局id
{
    int sym_index = -1 ;
    for (int i=0;i<GLOBAL_TABLE.size();i++) {
        if(GLOBAL_TABLE[i].first == sym_name) {
            sym_index = i;
            break;
        }
    }
    return sym_index;
}

void STORE_CONSTANTS(string id,string type,string value) //存储常量 全局/局部     局部的可以和全局的重名
{
    symbolTable info;
    info.type = type;
    info.nature = "CONST";
    //info.value = value;

    if(FUNC_MAP.empty()) { //全局 地址都由label决定
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //重定义名字
    } else { //局部 地址由sp + offset决定
            if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //当前函数为空
                info.offset = FUNC_MAP.back().second.offset; //无参数表，第一个常量的偏移量等于当前函数的偏移量
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            } else { //当前函数不为空
                if(FIND_IN_LOCAL(id) == -1) {
                    int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //上一个变/常量的偏移量
                    int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //上一个const的大小
                    int offset = last_offset + last_size;
                    info.offset = offset;
                    FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
                } else B_reDefName(); //重定义名字
            }
    }
}

void STORE_VARIABLES(string id,string type) //存储变量
{
    symbolTable info;
    info.type = type;
    info.nature = "VAR";

    if(FUNC_MAP.empty()) { //全局 地址都由label决定
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //重定义名字
    } else { //局部 地址由sp + offset决定
        if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //当前函数内的表为空
            info.offset = FUNC_MAP.back().second.offset; //无参数表，第一个变量的偏移量等于当前函数的偏移量
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else { //当前函数不为空
            if(FIND_IN_LOCAL(id) == -1) {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //上一个var的偏移量
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //上一个var的大小
                int offset = last_offset + last_size;
                info.offset = offset;
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            } else B_reDefName(); //重定义名字
        }
    }
}

void STORE_ARRAY(string id,string type,string arr_size) //存储数组
{
    symbolTable info;
    info.type = type;
    info.nature = "ARRAY";

    int res;
    stringstream ss;
    ss << arr_size;
    ss >> res;

    info.sym_size = res * 4;

    if(FUNC_MAP.empty()) { //全局 地址都由label决定
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //重定义名字
    } else { //局部 地址由sp + offset决定
        if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //当前函数内的表为空
            info.offset = FUNC_MAP.back().second.offset; //无参数表，第一个变量的偏移量等于当前函数的偏移量
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else { //当前函数不为空
            if(FIND_IN_LOCAL(id) == -1) {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //上一个var的偏移量
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //上一个var的大小
                int offset = last_offset + last_size;
                info.offset = offset;
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            }else B_reDefName(); //重定义名字
        }
    }
}

void STORE_FUNC(string id,funcInfo info) //存储函数 如果这个函数和变量重名，还是要存入符号表里，和其他函数重名，改一下名字存入符号表里
{
        int offset;
        if(FUNC_MAP.empty()) {
            if(FIND_IN_GLOBAL(id) == 1) B_reDefName(); //重定义名字
            offset = 0;
            info.offset = offset;
            FUNC_MAP.push_back(make_pair(id, info)); //不需要make_pair<string, funcInfo>
        } else {
            int offset;
            if(FIND_IN_GLOBAL(id) == 1 || GET_FUNC_INDEX(id) != -1) {
                B_reDefName(); //重定义名字
                if(GET_FUNC_INDEX(id) != -1) id = "RE" + id;
            }
            if (FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //上一个函数里p都没声明
                offset = FUNC_MAP.back().second.offset;
            } else {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //上一个函数中最后一个变/常量的偏移量
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //上一个函数中最后一个变/常量的大小
                offset = last_offset + last_size;
            }
            info.offset = offset;
            FUNC_MAP.push_back(make_pair(id, info)); //不需要make_pair<string, funcInfo>
        }
}

void STORE_PARATABLE(string id,string type) //存储函数的参数表
{
    symbolTable info;
    info.type = type;
    info.nature = "PARA";

    FUNC_MAP.back().second.para_cnt ++;

    //局部 地址由sp + offset决定
    if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //当前函数为空
        info.offset = FUNC_MAP.back().second.offset; //参数表第一个元素的偏移量等于当前函数的偏移量
        FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
    } else { //当前函数不为空
        if(FIND_IN_LOCAL(id) == -1) {
            int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //上一个var的偏移量
            int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //上一个var的大小
            int offset = last_offset + last_size;
            info.offset = offset;
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else B_reDefName(); //重定义名字
    }
}

