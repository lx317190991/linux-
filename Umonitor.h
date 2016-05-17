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

//win表示显示窗口，data表示传入数据
//bloaddata为true表示要重载数据，bshow为true时表示把界面显示到屏幕，bactivate表示需要交互
typedef void (*MenuFunc)(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data);

typedef struct UserPtr
{
    MENU **upmenu;//指向上层菜单
    MENU *downmenu;//菜单项下面挂的菜单
    MenuFunc func;//回调函数，当菜单对应的一个查询命令，则调用此回调函数
    char *cmd;//回调命令，当菜单执行的不是查询命令，并且需要打开一个新窗口的时候，调用此命令
    char *title;//分割的值的所有标题
    int *separator;//分割之后的值存放在此数组中，以NULL结束
}UserPtr;

#endif
