#include "UmessageGroup.h"

#include "Bmco/Util/Application.h"
#include "BolCommon/ControlRegionOp.h"
#include "BolCommon/MetaMessageQueueGroup.h"
#include "BolCommon/MetaMessageQueueGroupOp.h"
#include "Bmco/DateTime.h"
#include "Bmco/DateTimeFormatter.h"
#include "Bmco/DateTimeFormat.h"

#include "Bmco/Util/ServerApplication.h"
#include "Bmco/Util/Option.h"
#include "Bmco/Util/OptionSet.h"
#include "Bmco/Util/HelpFormatter.h"
#include "Bmco/Util/AbstractConfiguration.h"
#include "Bmco/NumberParser.h"
//#include "Bmco/logger.h"


using Bmco::DateTimeFormatter;
using Bmco::DateTimeFormat;
using Bmco::Util::Application;
using Bmco::DateTime;
using Bmco::Logger;

#include <string>
//#include <iostream>
#include <vector>
//#include <algorithm>

using namespace std;
using namespace Bmco;

//本界面上用到的ncurses-cdk的组件
static CDKSCREEN *l_cdkscreen    = 0; //cdk屏幕
static CDKSELECTION *l_selection = 0; //cdk多选列表控件
static CDKENTRY *l_entryID        = 0; //id输入表单控件
static CDKENTRY *l_entryFlowID  = 0; //name输入表单控件
static CDKDIALOG *l_dialog       = 0; //功能选项对话框控件
//本界面上的数据指针
static char **l_item             = 0;//多选列表数据指针
//static vector<BM35::MetaMessageQueue> l_vecMessageInfo;
static UserPtr *l_uptr           = 0;//多选列表标题和分割数组长度的结构体指针

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

namespace BM35{
static std::vector<MetaMessageQueueGroup> l_vecMessageInfo;
//把MetaBpcbInfo结构体信息以保存到字符串中返回，申请的内存由ConvertDataToString函数释放
static char *MessageInfoToString(const MetaMessageQueueGroup &mmq, const int *separator)
{
    
    Bmco::UInt32 i = 0;
    Bmco::UInt32 len = 0;//selection一行的长度
    Bmco::UInt32 maxItemLen = 0;//selection一行中所有项的最大长度
    
    while (0 != separator[i])
    {
        if (maxItemLen < separator[i])
        {
            maxItemLen = separator[i];
        }
        
        len = len + separator[i] + 1;
        i++;
    }
    
    char *str = new char[len+1];
    memset(str, ' ', len);
    str[len] = 0;
    char item[2048] = {0};
    i = 0;
    int nextItem = 0;

    snprintf(item,separator[i], "%d", mmq.ID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%s", mmq.Name.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    snprintf(item,separator[i], "%d", mmq.FlowID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    snprintf(item,separator[i], "%d", mmq.FlowStep);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    snprintf(item,separator[i], "%d", mmq.ProgramID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.MQCells);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.MQCellSize);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.MinMQNumber);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.MaxMQNumber);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.CurrentMQNumber);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    LocalDateTime dt(mmq.TimeStamp);
    std::string st = DateTimeFormatter::format(dt, DateTimeFormat::SORTABLE_FORMAT);
    snprintf(item,separator[i], "%s", st.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    return str;
}

//把vector中的数据按照separator中的分割宽度转换成字符串，保存在item中，返回item中的总项数
static int ConvertDataToString(char ***item, const std::vector<MetaMessageQueueGroup> &vecInfo, const int *separator)
{
    int count = 0;
    
    if (NULL != *item)
    {
         CDKfreeStrings(*item);
         *item = NULL;
    }

    int size = vecInfo.size();

    if (0 >= size)
    {
         return 0;
    }
    
    *item = (char **)malloc((size+1) * sizeof(*(*item)));

    int i;
    for (i=0; i<size; i++)
    {
        (*item)[i] = MessageInfoToString(vecInfo[i], separator);
        count++;
    }
    
    (*item)[i] = 0;

    return count;
}

//功能:从内存或表中查询数据，结果以字符串数组形式保存在item中
//输入:separator表示分割宽度数组，msgid和flowid表示查询条件
//输出:item保存查询结果
//返回:查询到的数据条数
static int LoadData(char ***item, const int *separator, int msgid, int flowid)
{
    int count = 0;


    MetaMessageQueueGroupOp* MsgQueuePtr = NULL;
    std::vector<MetaMessageQueueGroup> vecMsgQueueGroupInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    if (!ctlOptPtr->initialize())
    {
        return count;
    }
    
    MsgQueuePtr = dynamic_cast<MetaMessageQueueGroupOp*>(ctlOptPtr->getObjectPtr(MetaMessageQueueGroupOp::getObjName()));

    if (false == MsgQueuePtr->queryAll(vecMsgQueueGroupInfo))
    {
        g_pApp->logger().information("Failed to execute getAllMsgQueueGroup on MetaShmMsgQueueGroupTable");
        return count;
    }

    if ((-1 == msgid) && (-1 == flowid))//载入所有数据
    {
        count = ConvertDataToString(item, vecMsgQueueGroupInfo, separator);
        l_vecMessageInfo = vecMsgQueueGroupInfo;
    }
    
    else if ((-1 != msgid) && (-1 == flowid))
    {
        l_vecMessageInfo.clear();
        for (int i = 0; i < vecMsgQueueGroupInfo.size(); i++)
        {
            if (msgid == vecMsgQueueGroupInfo[i].ID)
            {
                l_vecMessageInfo.push_back(vecMsgQueueGroupInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    else if ((-1 == msgid) && (-1 != flowid))
    {
        l_vecMessageInfo.clear();
        for (int i=0; i<vecMsgQueueGroupInfo.size(); i++)
        {
            if (flowid == vecMsgQueueGroupInfo[i].FlowID)
            {
                l_vecMessageInfo.push_back(vecMsgQueueGroupInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    else
    {
        l_vecMessageInfo.clear();
        for (int i=0; i<vecMsgQueueGroupInfo.size(); i++)
        {
            if ((msgid == vecMsgQueueGroupInfo[i].ID) && (flowid == vecMsgQueueGroupInfo[i].FlowID))
            {
                l_vecMessageInfo.push_back(vecMsgQueueGroupInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    
    return count;
}
}

static void show()
{
    if (NULL != l_entryID) drawCDKEntry(l_entryID, 0);
    if (NULL != l_entryFlowID) drawCDKEntry(l_entryFlowID, 0);
    if (NULL != l_selection) drawCDKSelection(l_selection, 0);
}

static bool reload()
{
    char tmp[64] = {0};
    int iFlowID, iBpcbID;
    bool ret = true;
    
    strcpy(tmp, getCDKEntryValue(l_entryID));
    
    if (0 == strlen(tmp))
    {
        iFlowID = -1;
    }
    else
    {
        iFlowID = atoi(tmp);
    }
    
    strcpy(tmp, getCDKEntryValue(l_entryFlowID));
    
    if (0 == strlen(tmp))
    {
        iBpcbID = -1;
    }
    else
    {
        iBpcbID = atoi(tmp);
    }
    
    int count = BM35::LoadData(&l_item, l_uptr->separator, iFlowID, iBpcbID);

    if (NULL != l_selection)
    {
        if (0 >= count)
        {
            ret = false;
            const char *tmp[]={" ", NULL};
            setCDKSelectionItems(l_selection, (CDK_CSTRING2)tmp, 1);
            l_selection->listSize = count;
        }
        setCDKSelectionItems(l_selection, (CDK_CSTRING2)(l_item), count);
        drawCDKSelection(l_selection, TRUE);
    }

    if (NULL != l_item)
    {
         CDKfreeStrings(l_item);
         l_item = NULL;
    }

    return ret;
}

static int SelectionFresh(EObjectType cdktype GCC_UNUSED,
                             void *object GCC_UNUSED, void *clientData, chtype key)
{
    reload();
    show();
    return (TRUE);
}

// 将以一定格式配置的字符串以一定间隔输出
static char* formatConvert(const int *separator, char *title)
{
    Bmco::Int32 i = 0;

    // title从可控区域第二位开始显示
    Bmco::Int32 nextItem = 2;

    //selection一行的长度
    Bmco::UInt32 len = 0;
    
    //selection一行中所有项的最大长度
    Bmco::UInt32 maxItemLen = 0;
    
    while (0 != separator[i])
    {   
        if (maxItemLen < separator[i])
        {
            maxItemLen = separator[i];
        }
        len = len + separator[i] + 1;
        i++;
    }
    i = 0;
    char *str = new char[len+1];
    memset(str, ' ', len);
    str[len] = 0;
    char item[2048] = {0};
    std::string os = title;
    Bmco::StringTokenizer tok(os, "|", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

    // 以'|'分隔分别对每一项进行指定间距显示
    for (StringTokenizer::Iterator it = tok.begin(); it != tok.end(); ++it)
    {
        snprintf(item,separator[i], "%s", (*it).c_str());
        if (separator[i] >= (*it).length())
        {
            memcpy(str+nextItem, item, (*it).length());
        }
        else
        {
            memcpy(str+nextItem, item, separator[i]-1);
        }
        nextItem += separator[i];
        i++;
    }

    return str; 
}


static bool init()
{

    static const char *choices[] =
    {
        " ",
        "</B/56>-><!B!56>"
    };

    if (!BM35::LoadData(&l_item, l_uptr->separator, -1, -1))
    {
        return false;
    }
    
    if (NULL != l_selection)
    {
        destroyCDKSelection(l_selection);
        l_selection = NULL;
    }

    
    l_uptr->title = formatConvert(l_uptr->separator, l_uptr->title);

    int count = 0;
    for (int i=0; 0!=l_item[i]; i++) count++;
    l_selection = newCDKSelection(l_cdkscreen,
                                    CENTER,
                                    5,
                                    RIGHT,
                                    LINES-6,
                                    COLS,
                                    l_uptr->title,
                                    (CDK_CSTRING2)l_item,
                                    count,
                                    (CDK_CSTRING2)choices,
                                    2,
                                    A_REVERSE,
                                    TRUE,
                                    FALSE);
    
    if (!l_selection) {return false;}
   
    if (NULL != l_entryID)
    {
        destroyCDKEntry(l_entryID);
        l_entryID = NULL;
    }

    l_entryID = newCDKEntry(l_cdkscreen,
                                1,
                                4,
                                NULL,
                                "ID: ",
                                A_UNDERLINE | COLOR_PAIR(56),
                                '_',
                                vINT,
                                16, 0, 64,
                                FALSE,
                                FALSE);
    
    if (!l_entryID) {return false;}
   
    if (NULL != l_entryFlowID)
    {
        destroyCDKEntry(l_entryFlowID);
        l_entryFlowID = NULL;
    }

    l_entryFlowID = newCDKEntry(l_cdkscreen,
                                30,
                                4,
                                NULL,
                                "FlowID: ",
                                A_UNDERLINE | COLOR_PAIR(56),
                                '_',
                                vINT,
                                16, 0, 64,
                                FALSE,
                                FALSE);

    if (!l_entryFlowID) {return false;}
        const char    *message[4];
        message[0] = "<C></U>choose Command Interface";
        message[1] = "Pick the command you wish to run and press enter!";
        //message[2] = "<C>Press </R>?<!R> for help.";
        const char *buttons[] =
        {"</B/24>1.刷新<!B!24>", "</B/24>2.实时刷新<!B!24>",
         "</B/16>q.退出<!B!16>"};
        
    l_dialog = newCDKDialog(l_cdkscreen,
                            CENTER,
                            CENTER,
                            (CDK_CSTRING2)message, 2,
                            (CDK_CSTRING2)buttons, 3,
                            COLOR_PAIR(2) | A_REVERSE,
                            TRUE,
                            TRUE,
                            FALSE);
    
    //bindCDKObject (vSELECTION, l_selection, '?', selectiondetail, NULL);
    //bindCDKObject (vSELECTION, l_selection, '/', selectionquery, NULL);
    bindCDKObject (vSELECTION, l_selection, KEY_F(5), SelectionFresh, NULL);

    if (NULL != l_item)
    {
        CDKfreeStrings(l_item);
        l_item = NULL;
    }
    
    return true;
}

static void traverse()
{
    while (true)
    {
        activateCDKEntry(l_entryID, NULL);

        if (KEY_ENTER == l_entryID->obj.input)
        {
            reload();
            show();
        }
        else
        {
            break;
        }
    }
bj:
    while (true)
    {
        activateCDKEntry(l_entryFlowID, NULL);

        if (KEY_ENTER == l_entryFlowID->obj.input)
        {
            reload();
            show();
        }
        else
        {
            break;
        }
    }

    //如果selection中没有内容，则跳过这个widget的导航
    if (0 >= getCDKSelectionItems(l_selection, NULL))
    {
        return;
    }
    
    while (true)
    {
        //Activate the selection list.
        activateCDKSelection(l_selection, 0);

        //Check the exit status of the widget.
        if (l_selection->exitType == vNORMAL)
        {
            if (KEY_ENTER == l_selection->obj.input)
            {
                int selection = activateCDKDialog(l_dialog, 0);
                eraseCDKDialog(l_dialog);

                if (l_dialog->exitType == vNORMAL)
                {
                    if (KEY_ENTER == l_dialog->obj.input)
                    {
                        switch (selection)
                        {
                            case 0://1.刷新
                            {
                                if (!reload())
                                {
                                    goto bj;
                                }
                                break;
                            }
                            case 1://2.实时刷新
                            {
                                halfdelay(10);
                                while(ERR == getch())
                                {
                                    reload();
                                }
                                nocbreak();
                                cbreak();
                                break;
                            }
                            case 2://3.日志
                            {
                                break;
                            }
                            case 3://q.退出
                            {
                                break;
                            }
                            default:
                            {
                            }
                        }
                    }
                }
                
                //eraseCDKDialog(l_dialog);
            }
            else
            {
                break;
            } 
        }
    }
}

//释放界面占用内存
static void UmessageGroupDestory()
{
    if (NULL != l_selection)
    {
        destroyCDKSelection(l_selection);
        l_selection = NULL;
    }
    if (NULL != l_entryID)
    {
        destroyCDKEntry(l_entryID);
        l_entryID = NULL;
    }

    if (NULL != l_item)
    {
        CDKfreeStrings(l_item);
        l_item = NULL;
    }

    if (l_cdkscreen)
    {
        destroyCDKScreen(l_cdkscreen);
        l_cdkscreen = NULL;
    }
}

//消息队列管理接口函数
void UmessageGroup(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data)
{
    if (NULL == win)
    {
        UmessageGroupDestory();
        return;
    }
    
    if (NULL == l_cdkscreen)
    {
        if (NULL == data) return;
        l_uptr = (UserPtr *)data;
        l_cdkscreen = initCDKScreen(win);
        
        if (NULL != l_cdkscreen)
        {
            if (!init())
            {
                destroyCDKScreen(l_cdkscreen);
                l_cdkscreen = NULL;
                return;
            }
        }
        else
        {
            return;
        }
    }

    if (true == bloaddata)
    {
        reload();
    }

    if (true==bshow)
    {
        show();
    }
    
    if (true == bactivate)
    {
        traverse();
    }
    
}

