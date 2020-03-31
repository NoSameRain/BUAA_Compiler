#ifndef ERRORHANDLER_H_INCLUDED
#define ERRORHANDLER_H_INCLUDED

extern int lineNum; // which line in file now
extern int flag_read; // flag = 0 -- read ahead now / flag = 1 -- read now
extern int READ_N; //����get����sym������һ��sym���˼���
extern int LINE_ADD; //ͬ��
extern int LACK_PARENT;//��������ʱ����һλ

extern vector <string> errorVec; // �����������

void A_illIDNFR(char s); // �Ƿ���ʶ��
void A_illChar(char s); //�Ƿ��ַ�
void A_unMatchQuates(); //���Ų�ƥ��
void B_reDefName();
void C_unDefName(int i,string id);
void D_E_valParaTableError(string id,vector <string> VPT); //�����������������Ͳ�ƥ��
void F_IllTypeInCondition(); //�����ж��г��ֲ��Ϸ�����
void G_Return_Surplus();
void H_Return_Lack();
void H_Return_unMatch(string ret_name); //����ֵ���Ͳ�ƥ��
void I_ARRAY_INDEX_ERROR(string index_name); //�����±�ӦΪ���ͱ��ʽ
void J_CHANGE_CONST_VALUE(string id); //�ı䳣����ֵ
void K_SHOULD_BE_SEMICN(); //ȱ�ٷֺ�
void L_LACK_RPARENT(); //ȱ����С����
void M_LACK_RBRACK(); //ȱ����������
void N_LACK_WHILE(); //do whileȱ��while
void O_constDefTypeError(); //����������=����ֻ�������ͻ��ַ��ͳ���
void error(string a);
void printError(string a);

#endif // ERRORHANDLER_H_INCLUDED
