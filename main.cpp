#include "header.h"
#include "interCode.h"
#include "errorHandler.h"

using namespace std;

ifstream codefile;
ofstream outfileE,outfileM,outfileIn;

int codeN,ind;
int cateInd;
int cate; // cateInd ��Ԥ��
char code[MAX_code];
char g_word[MAX_word]; //Ԥ������
string s_word; //Ŀǰ�Ͽɵĵ���
string callName;

vector <int> re_Track; // ÿ����һ��getsym,����һ��ind�����������vec

int main()
{
    codefile.open("testfile.txt", ios::binary);
    //outfileE.open("error.txt",ios::out);
    outfileM.open("mips.txt",ios::out);
    //outfileIn.open("inter.txt",ios::out);

    char a;
    while((a = codefile.get()) != EOF) {
        code[codeN++] = a;
    }

    jSyntax(); //�﷨����

    //if(!errorVec.empty()) {
        //for(std::vector<string>::iterator iter = errorVec.begin();
        //iter != errorVec.end(); iter++ ) {
            //outfileE << *iter << endl;
            //cout << *iter << endl;
        //}
    //} else {
        gene_code(); //����mips

        /*for(int i = 0;i < intercode.size();i++) {
            for(int j = 0;j < intercode[i].size();j++) {
                cout << intercode[i][j]<< " ";
            }
            cout << "" << endl;
        }*/

        for(int i = 0;i < mipscode.size();i++) {
            outfileM << mipscode[i] << endl;
        }
    //}

    codefile.close();
    //outfileE.close();
    outfileM.close();
    return 0;
}





