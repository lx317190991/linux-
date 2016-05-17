#ifndef _UMONITOR_H_
#define _UMONITOR_H_
#include <ncurses.h>
#include <cdk.h>
#include <panel.h>
#include <menu.h>
#include <form.h>

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "UmonitorApp.h"

#include "Markup.h"

#include "BprocessBOL.h"
#include "Umessage.h"
#include "Umem.h"
#include "UmessageGroup.h"
#include "UregionMem.h"
#include "UprogramDef.h"
#include "UbpcbInfo.h"
#include "UregularProcessTask.h"
#include "UMQDef.h"
#include "UMQRela.h"
#include "UMQTopic.h"
#include "UMQService.h"

extern BM35::UmonitorApp *g_pApp;
int UmonitorMain(const std::vector<std::string>& args, BM35::UmonitorApp *pApp);

//win��ʾ��ʾ���ڣ�data��ʾ��������
//bloaddataΪtrue��ʾҪ�������ݣ�bshowΪtrueʱ��ʾ�ѽ�����ʾ����Ļ��bactivate��ʾ��Ҫ����
typedef void (*MenuFunc)(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data);

typedef struct UserPtr
{
    MENU **upmenu;//ָ���ϲ�˵�
    MENU *downmenu;//�˵�������ҵĲ˵�
    MenuFunc func;//�ص����������˵���Ӧ��һ����ѯ�������ô˻ص�����
    char *cmd;//�ص�������˵�ִ�еĲ��ǲ�ѯ���������Ҫ��һ���´��ڵ�ʱ�򣬵��ô�����
    char *title;//�ָ��ֵ�����б���
    int *separator;//�ָ�֮���ֵ����ڴ������У���NULL����
}UserPtr;

#endif
