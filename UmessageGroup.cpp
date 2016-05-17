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

//���������õ���ncurses-cdk�����
static CDKSCREEN *l_cdkscreen    = 0; //cdk��Ļ
static CDKSELECTION *l_selection = 0; //cdk��ѡ�б�ؼ�
static CDKENTRY *l_entryID        = 0; //id������ؼ�
static CDKENTRY *l_entryFlowID  = 0; //name������ؼ�
static CDKDIALOG *l_dialog       = 0; //����ѡ��Ի���ؼ�
//�������ϵ�����ָ��
static char **l_item             = 0;//��ѡ�б�����ָ��
//static vector<BM35::MetaMessageQueue> l_vecMessageInfo;
static UserPtr *l_uptr           = 0;//��ѡ�б����ͷָ����鳤�ȵĽṹ��ָ��

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

namespace BM35{
static std::vector<MetaMessageQueueGroup> l_vecMessageInfo;
//��MetaBpcbInfo�ṹ����Ϣ�Ա��浽�ַ����з��أ�������ڴ���ConvertDataToString�����ͷ�
static char *MessageInfoToString(const MetaMessageQueueGroup &mmq, const int *separator)
{
    
    Bmco::UInt32 i = 0;
    Bmco::UInt32 len = 0;//selectionһ�еĳ���
    Bmco::UInt32 maxItemLen = 0;//selectionһ�������������󳤶�
    
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

//��vector�е����ݰ���separator�еķָ���ת�����ַ�����������item�У�����item�е�������
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

//����:���ڴ����в�ѯ���ݣ�������ַ���������ʽ������item��
//����:separator��ʾ�ָ������飬msgid��flowid��ʾ��ѯ����
//���:item�����ѯ���
//����:��ѯ������������
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

    if ((-1 == msgid) && (-1 == flowid))//������������
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

// ����һ����ʽ���õ��ַ�����һ��������
static char* formatConvert(const int *separator, char *title)
{
    Bmco::Int32 i = 0;

    // title�ӿɿ�����ڶ�λ��ʼ��ʾ
    Bmco::Int32 nextItem = 2;

    //selectionһ�еĳ���
    Bmco::UInt32 len = 0;
    
    //selectionһ�������������󳤶�
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

    // ��'|'�ָ��ֱ��ÿһ�����ָ�������ʾ
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
        {"</B/24>1.ˢ��<!B!24>", "</B/24>2.ʵʱˢ��<!B!24>",
         "</B/16>q.�˳�<!B!16>"};
        
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

    //���selection��û�����ݣ����������widget�ĵ���
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
                            case 0://1.ˢ��
                            {
                                if (!reload())
                                {
                                    goto bj;
                                }
                                break;
                            }
                            case 1://2.ʵʱˢ��
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
                            case 2://3.��־
                            {
                                break;
                            }
                            case 3://q.�˳�
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

//�ͷŽ���ռ���ڴ�
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

//��Ϣ���й���ӿں���
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

