#ifndef _BPROCESSBOL_H_
#define _BPROCESSBOL_H_
#include "Umonitor.h"

//win��ʾ��ʾ���ڣ�data��ʾ��������
//bloaddataΪtrue��ʾҪ�������ݣ�bshowΪtrueʱ��ʾ�ѽ�����ʾ����Ļ��bactivate��ʾ��Ҫ����
void BprocessBOL(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data);
//֧�ֶ����ݵ�ȫѡ����ȫѡ
int ChoiceAll(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key);
//֧�ֶ����ݵĲ���ѡ��
int ChoicesPart(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key);

#endif
