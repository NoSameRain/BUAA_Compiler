#include "header.h"
#include "mapStore.h"
#include "errorHandler.h"

vector<pair<string, symbolTable>> GLOBAL_TABLE; //ȫ�ֳ������� id + info
vector<pair<string, funcInfo>> FUNC_MAP; // ������(id + info) //FUNC_MAP.back().

int FIND_IN_GLOBAL(string id) //��ȫ�ֱ� ��������
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

int FIND_IN_LOCAL(string id) //�ڵ�ǰ����������Ĳ����䳣������
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

int GET_SYM_INDEX(int func_index,string sym_name) //�õ�ָ�������е�ָ��id û�ҵ�����-1
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

int GET_GLOBAL_SYM_INDEX(string sym_name) //�õ�ָ����ȫ��id
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

void STORE_CONSTANTS(string id,string type,string value) //�洢���� ȫ��/�ֲ�     �ֲ��Ŀ��Ժ�ȫ�ֵ�����
{
    symbolTable info;
    info.type = type;
    info.nature = "CONST";
    //info.value = value;

    if(FUNC_MAP.empty()) { //ȫ�� ��ַ����label����
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //�ض�������
    } else { //�ֲ� ��ַ��sp + offset����
            if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //��ǰ����Ϊ��
                info.offset = FUNC_MAP.back().second.offset; //�޲�������һ��������ƫ�������ڵ�ǰ������ƫ����
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            } else { //��ǰ������Ϊ��
                if(FIND_IN_LOCAL(id) == -1) {
                    int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //��һ����/������ƫ����
                    int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //��һ��const�Ĵ�С
                    int offset = last_offset + last_size;
                    info.offset = offset;
                    FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
                } else B_reDefName(); //�ض�������
            }
    }
}

void STORE_VARIABLES(string id,string type) //�洢����
{
    symbolTable info;
    info.type = type;
    info.nature = "VAR";

    if(FUNC_MAP.empty()) { //ȫ�� ��ַ����label����
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //�ض�������
    } else { //�ֲ� ��ַ��sp + offset����
        if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //��ǰ�����ڵı�Ϊ��
            info.offset = FUNC_MAP.back().second.offset; //�޲�������һ��������ƫ�������ڵ�ǰ������ƫ����
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else { //��ǰ������Ϊ��
            if(FIND_IN_LOCAL(id) == -1) {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //��һ��var��ƫ����
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //��һ��var�Ĵ�С
                int offset = last_offset + last_size;
                info.offset = offset;
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            } else B_reDefName(); //�ض�������
        }
    }
}

void STORE_ARRAY(string id,string type,string arr_size) //�洢����
{
    symbolTable info;
    info.type = type;
    info.nature = "ARRAY";

    int res;
    stringstream ss;
    ss << arr_size;
    ss >> res;

    info.sym_size = res * 4;

    if(FUNC_MAP.empty()) { //ȫ�� ��ַ����label����
        if(FIND_IN_GLOBAL(id) == 0) GLOBAL_TABLE.push_back(make_pair(id, info));
        else B_reDefName(); //�ض�������
    } else { //�ֲ� ��ַ��sp + offset����
        if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //��ǰ�����ڵı�Ϊ��
            info.offset = FUNC_MAP.back().second.offset; //�޲�������һ��������ƫ�������ڵ�ǰ������ƫ����
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else { //��ǰ������Ϊ��
            if(FIND_IN_LOCAL(id) == -1) {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //��һ��var��ƫ����
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //��һ��var�Ĵ�С
                int offset = last_offset + last_size;
                info.offset = offset;
                FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
            }else B_reDefName(); //�ض�������
        }
    }
}

void STORE_FUNC(string id,funcInfo info) //�洢���� �����������ͱ�������������Ҫ������ű��������������������һ�����ִ�����ű���
{
        int offset;
        if(FUNC_MAP.empty()) {
            if(FIND_IN_GLOBAL(id) == 1) B_reDefName(); //�ض�������
            offset = 0;
            info.offset = offset;
            FUNC_MAP.push_back(make_pair(id, info)); //����Ҫmake_pair<string, funcInfo>
        } else {
            int offset;
            if(FIND_IN_GLOBAL(id) == 1 || GET_FUNC_INDEX(id) != -1) {
                B_reDefName(); //�ض�������
                if(GET_FUNC_INDEX(id) != -1) id = "RE" + id;
            }
            if (FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //��һ��������p��û����
                offset = FUNC_MAP.back().second.offset;
            } else {
                int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //��һ�����������һ����/������ƫ����
                int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //��һ�����������һ����/�����Ĵ�С
                offset = last_offset + last_size;
            }
            info.offset = offset;
            FUNC_MAP.push_back(make_pair(id, info)); //����Ҫmake_pair<string, funcInfo>
        }
}

void STORE_PARATABLE(string id,string type) //�洢�����Ĳ�����
{
    symbolTable info;
    info.type = type;
    info.nature = "PARA";

    FUNC_MAP.back().second.para_cnt ++;

    //�ֲ� ��ַ��sp + offset����
    if(FUNC_MAP.back().second.LOCAL_TABLE.empty()) { //��ǰ����Ϊ��
        info.offset = FUNC_MAP.back().second.offset; //�������һ��Ԫ�ص�ƫ�������ڵ�ǰ������ƫ����
        FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
    } else { //��ǰ������Ϊ��
        if(FIND_IN_LOCAL(id) == -1) {
            int last_offset = FUNC_MAP.back().second.LOCAL_TABLE.back().second.offset; //��һ��var��ƫ����
            int last_size = FUNC_MAP.back().second.LOCAL_TABLE.back().second.sym_size; //��һ��var�Ĵ�С
            int offset = last_offset + last_size;
            info.offset = offset;
            FUNC_MAP.back().second.LOCAL_TABLE.push_back(make_pair(id, info));
        } else B_reDefName(); //�ض�������
    }
}

