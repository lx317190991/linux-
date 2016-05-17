#ifndef _BPROCESSBOL_H_
#define _BPROCESSBOL_H_
#include "Umonitor.h"

//win表示显示窗口，data表示传入数据
//bloaddata为true表示要重载数据，bshow为true时表示把界面显示到屏幕，bactivate表示需要交互
void BprocessBOL(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data);
//支持对内容的全选，反全选
int ChoiceAll(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key);
//支持对内容的部分选择
int ChoicesPart(EObjectType cdktype GCC_UNUSED, void *object GCC_UNUSED, void *clientData, chtype key);

#endif
