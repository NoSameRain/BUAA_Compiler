#include "header.h"
#include "errorHandler.h"

int lineNum = 1; // which line in file now
int flag_read = 0;
int READ_N; //本次get到的sym距离上一个sym换了几行
int LINE_ADD; //同上

void jLexical()
{
    int retrack = 0;
    READ_N = 0;

    initial();

    while(isspace(code[ind]) && ind < codeN){
        if (code[ind] == '\n') {
            if(flag_read){
                lineNum++; //正式读时line change
                READ_N++;//本次get到的sym距离上一个sym换了几行
            }
        }
        ind++;
        retrack++;
    }

    if(isalnum(code[ind]) || code[ind] == '_') { //IDENFR,INTCON,13reservedWord
        int i = 0;
        int flag = 0;
        int flag_num = 1; //judge INTCON

        while((isalnum(code[ind]) || code[ind] == '_') && ind < codeN) {
            if(isalpha(code[ind]) || code[ind] == '_') flag_num = 0; //not intcon
            if (isdigit(code[ind]) || code[ind] == '_') flag = 1;
            g_word[i++] = code[ind++];
            retrack++;
        }
        g_word[i] = '\0';
        if(flag_num) {
            cate = 1; //INTCON
        } else {
            if(flag_read) A_illIDNFR(g_word[0]); // 非法标识符
            cate = 0; //IDENFR
            if(flag == 0) JudgeReserWord(); //13reservedWord
        }
    } else if(code[ind] == '\'') {
        int flag = 1;
        ind++; retrack++;
        g_word[0] = code[ind++]; retrack++;

            if (flag_read) {
                if(code[ind] != '\'') {
                    flag = 0;
                    A_unMatchQuates(); //引号不匹配
                }
                A_illChar(g_word[0]);
            }
            cate = 2; //CHARCON
            if(flag) {
                ind++; retrack++;
            }
    } else if(code[ind] == '\"') {
        int i = 0;
        int flag = 1;
        ind++; retrack++;
        while(code[ind] != '\"' && ind < codeN) {
            //if (flag_read && (code[ind] == ',' || code[ind] == ')')) {
                //flag = 0;
                //A_unMatchQuates(); //引号不匹配
                //break;
            //}
            g_word[i++] = code[ind++]; retrack++;
        }
        g_word[i++] = '\0';
        cate = 3; //STRCON
        if(flag) {
            ind++; retrack++;
        }
    } else if(code[ind] == '+') {
        g_word[0] = code[ind++]; retrack++;
        cate = 17; //PLUS
    } else if(code[ind] == '-') {
        g_word[0] = code[ind++]; retrack++;
        cate = 18; //MINU
    } else if(code[ind] == '*') {
        g_word[0] = code[ind++]; retrack++;
        cate = 19; //MULT
    } else if(code[ind] == '/') {
        g_word[0] = code[ind++]; retrack++;
        cate = 20; //DIV
    } else if(code[ind] == '<') {
        g_word[0] = code[ind++]; retrack++;
        cate = 21; //LSS
        if(code[ind] == '=') {
            g_word[1] = code[ind++]; retrack++;
            g_word[2] = '\0';
            cate = 22; //LEQ
        }
    } else if(code[ind] == '>') {
        g_word[0] = code[ind++]; retrack++;
        cate = 23; //GRE
        if(code[ind] == '=') {
            g_word[1] = code[ind++]; retrack++;
            g_word[2] = '\0';
            cate = 24; //GEQ
        }
    } else if(code[ind] == '=') {
        g_word[0] = code[ind++]; retrack++;
        cate = 27; //ASSIGN
        if(code[ind] == '=') {
            g_word[1] = code[ind++]; retrack++;
            g_word[2] = '\0';
            cate = 25; //EQL
        }
    } else if(code[ind] == '!') {
        g_word[0] = code[ind++]; retrack++;
        g_word[1] = code[ind++]; retrack++;
        cate = 26; //NEQ
    } else if(code[ind] == ';') {
        g_word[0] = code[ind++]; retrack++;
        cate = 28; //SEMICN
    } else if(code[ind] == ',') {
        g_word[0] = code[ind++]; retrack++;
        cate = 29; //COMMA
    } else if(code[ind] == '(') {
        g_word[0] = code[ind++]; retrack++;
        cate = 30; //LPARENT
    } else if(code[ind] == ')') {
        g_word[0] = code[ind++]; retrack++;
        cate = 31; //RPARENT
    } else if(code[ind] == '[') {
        g_word[0] = code[ind++]; retrack++;
        cate = 32; //LBRACK
    } else if(code[ind] == ']') {
        g_word[0] = code[ind++]; retrack++;
        cate = 33; //RBRACK
    } else if(code[ind] == '{') {
        g_word[0] = code[ind++]; retrack++;
        cate = 34; //LBRACE
    } else if(code[ind] == '}') {
        g_word[0] = code[ind++]; retrack++;
        cate = 35; //RBRACE
    } else{
        if (flag_read) printError("a"); // 不符合词法
        retrack++; ind++;
    }

    LINE_ADD = READ_N;

    re_Track.push_back(retrack);
}

void getSym()
{
    flag_read = 1; //正式读
    jLexical();
    flag_read = 0; //预读

    /*for(int i = 0;i < sizeof(s_word);i++){
        s_word[i] = '\0';
    }*/
    s_word = g_word;
    cateInd = cate; //全局 = 预读
}

void JudgeReserWord()
{
    int i;

    for (i = 0;i < 13;i++) {
        if(reserWord[i] == g_word){
            cate = i + 4;
            break;
        }
    }
}

void initial()
{
    for(int i = 0;i < sizeof(g_word);i++){
        g_word[i] = '\0';
    }
}
