#include "Umessage.h"

#include "Bmco/Util/Application.h"
#include "BolCommon/ControlRegionOp.h"
#include "BolCommon/MetaMessageQueueInfo.h"
#include "BolCommon/MetaMessageQueueInfoOp.h"
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
#include "BolCommon/MessageQueue.h"
#include <vector>

using namespace std;
using namespace Bmco;

//���������õ���ncurses-cdk�����
static CDKSCREEN *l_cdkscreen    = 0; //cdk��Ļ
static CDKSELECTION *l_selection = 0; //cdk��ѡ�б�ؼ�
static CDKENTRY *l_entryID        = 0; //id������ؼ�
static CDKENTRY *l_entryGroupID  = 0; //name������ؼ�
static CDKDIALOG *l_dialog       = 0; //����ѡ��Ի���ؼ�
//�������ϵ�����ָ��
static char **l_item             = 0;//��ѡ�б�����ָ��
//static vector<BM35::MetaMessageQueue> l_vecMessageInfo;
static UserPtr *l_uptr           = 0;//��ѡ�б����ͷָ����鳤�ȵĽṹ��ָ��

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

namespace BM35{
static std::vector<MetaMessageQueue> l_vecMessageInfo;
//��MetaBpcbInfo�ṹ����Ϣ�Ա��浽�ַ����з��أ�������ڴ���ConvertDataToString�����ͷ�
static char *MessageInfoToString(const MetaMessageQueue &mmq, const int *separator)
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
    char item[2048] = {0};;
    i = 0;
    int nextItem = 0;

    snprintf(item,separator[i], "%d", mmq.ID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.MQGroupID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%s", mmq.Name.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    mmq.IsDynamic ? strncpy(item, "true",separator[i]) : strncpy(item, "false",separator[i]);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    
    snprintf(item, separator[i],"%d", mmq.CellNumber);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.CellBytes);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    Bmco::UInt32 usedCells = 0;
    MessageQueue MsgQueue;
    try
    {
        if (MsgQueue.attach(mmq.Name.c_str()))
        {
            if (mmq.State == MESSAGE_QUEUE_ACTIVE)
            {
                usedCells = MsgQueue.getMsgCurNum();
            }
            else
            {
                usedCells = mmq.UsedCells;
            }
        }
    }
    catch (...)
    {
        usedCells = 0;
    }
    snprintf(item,separator[i], "%d", usedCells);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mmq.State);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%s", mmq.MutexName.c_str());
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
static int ConvertDataToString(char ***item, const std::vector<MetaMessageQueue> &vecInfo, const int *separator)
{
    int count = 0;

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
//����:separator��ʾ�ָ������飬msgid��groupid��ʾ��ѯ����
//���:item�����ѯ���
//����:��ѯ������������
static int LoadData(char ***item, const int *separator, int msgid, int groupid)
{
    int count = 0;

    MetaMessageQueueInfoOp* MsgQueuePtr = NULL;
    std::vector<MetaMessageQueue> vecMsgQueueInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    if (ctlOptPtr->initialize())
    {
        MsgQueuePtr = dynamic_cast<MetaMessageQueueInfoOp*>(ctlOptPtr->getObjectPtr(MetaMessageQueueInfoOp::getObjName()));
    }
    else
    {
        return count;
    }

    if (false == MsgQueuePtr->queryAll(vecMsgQueueInfo))
    {
        g_pApp->logger().information("Failed to execute getAllMsgQueueInfo on MetaShmMsgQueueInfoTable");
        return count;
    }

    if ((-1 == msgid) && (-1 == groupid))//������������
    {
        count = ConvertDataToString(item, vecMsgQueueInfo, separator);
        l_vecMessageInfo = vecMsgQueueInfo;
    }
    
    else if ((-1 != msgid) && (-1 == groupid))
    {
        l_vecMessageInfo.clear();
        for (int i = 0; i < vecMsgQueueInfo.size(); i++)
        {
            if (msgid == vecMsgQueueInfo[i].ID)
            {
                l_vecMessageInfo.push_back(vecMsgQueueInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    else if ((-1 == msgid) && (-1 != groupid))
    {
        l_vecMessageInfo.clear();
        for (int i=0; i<vecMsgQueueInfo.size(); i++)
        {
            if (groupid == vecMsgQueueInfo[i].MQGroupID)
            {
                l_vecMessageInfo.push_back(vecMsgQueueInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    else
    {
        l_vecMessageInfo.clear();
        for (int i=0; i<vecMsgQueueInfo.size(); i++)
        {
            if ((msgid == vecMsgQueueInfo[i].ID) && (groupid == vecMsgQueueInfo[i].MQGroupID))
            {
                l_vecMessageInfo.push_back(vecMsgQueueInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMessageInfo, separator);
    }
    
    return count;
}

//ָֹͣ��bpcbID�Ľ��� -c -pѡ��
/*static bool DeleteMessageByID(int ID)
{
    MetaMessageQueueInfoOp* msgQueuePtr = NULL;
    std::vector<MetaMessageQueue> vecMsgQueueInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::CtlOptPtr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    MetaMsgTaskListOp* msgTaskPtr = dynamic_cast<MetaMsgTaskListOp*>(ctlOptPtr->getObjectPtr(MetaMsgTaskListOp::getObjName()));
    
    if (!ctlOptPtr->initialize())
    {
        g_pApp->logger().information("Failed to execute initialize on MetaShmBpcbInfoTable");
        return false;
    }

    Bmco::Timestamp now;
    MetaMsgTaskList::Ptr payload
                (new MetaMsgTaskList(ID, DELETE_MESSAGQUEUE, 0, 0, 0, true, 10, 10, "deleteMsgQueue", now));
    if (false == msgTaskPtr->Delete(payload, MsgTaskListTableIDIdx_t))
    {
        g_pApp->logger().information("Failed to execute delete on MetaShmMsgTaskListTable");
        return false;
    }

    return true;
}

static bool CreateMessageByID()
{
    MetaMessageQueueInfoOp* msgQueuePtr = NULL;
    std::vector<MetaMessageQueue> vecMsgQueueInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::CtlOptPtr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    MetaMsgTaskListOp* msgTaskPtr = dynamic_cast<MetaMsgTaskListOp*>(ctlOptPtr->getObjectPtr(MetaMsgTaskListOp::getObjName()));
    
    if (!ctlOptPtr->initialize())
    {
        g_pApp->logger().information("Failed to execute initialize on MetaShmBpcbInfoTable");
        return false;
    }
    
    Bmco::Timestamp now;
    MetaMsgTaskList::Ptr payload
                (new MetaMsgTaskList(0, CREATE_MESSAGQUEUE, 0, 0, 0, true, 10, 10, "createMsgQueue", now));
    if (false == msgTaskPtr->Insert(payload))
    {
        g_pApp->logger().information("Failed to execute Insert on MetaShmMsgTaskListTable");
        return false;
    }

    return true;
}*/
}

static void show()
{
    //bool ret = false;
    if (NULL != l_entryID) drawCDKEntry(l_entryID, 0);
    if (NULL != l_entryGroupID) drawCDKEntry(l_entryGroupID, 0);
    if (NULL != l_selection) drawCDKSelection(l_selection, 0);
    //if (NULL != l_cdkscreen) refreshCDKScreen(l_cdkscreen);
    //ret = true;
    //return ret;
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
    
    strcpy(tmp, getCDKEntryValue(l_entryGroupID));
    
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

//�û������̵�F5��ˢ�£��൱����������һ������
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

    const char *choices[] =
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
    
    if (NULL != l_entryGroupID)
    {
        destroyCDKEntry(l_entryGroupID);
        l_entryGroupID = NULL;
    }

    l_entryGroupID = newCDKEntry(l_cdkscreen,
                                30,
                                4,
                                NULL,
                                "GroupID: ",
                                A_UNDERLINE | COLOR_PAIR(56),
                                '_',
                                vINT,
                                16, 0, 64,
                                FALSE,
                                FALSE);

    if (!l_entryGroupID) {return false;}

    const char *message[4];
    message[0] = "<C></U>choose Command Interface";
    message[1] = "Pick the command you wish to run and press enter!";
    const char *buttons[] =
    {"</B/24>1.ˢ��<!B!24>", "</B/24>2.ʵʱˢ��<!B!24>", "</B/16>q.�˳�<!B!16>"};
        
    l_dialog = newCDKDialog(l_cdkscreen,
                            CENTER,
                            CENTER,
                            (CDK_CSTRING2)message, 2,
                            (CDK_CSTRING2)buttons, 3,
                            COLOR_PAIR(2) | A_REVERSE,
                            TRUE,
                            TRUE,
                            FALSE);

    if (!l_dialog) {return false;}
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
        activateCDKEntry(l_entryGroupID, NULL);

        if (KEY_ENTER == l_entryGroupID->obj.input)
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
static void UmessageDestory()
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

    if (NULL != l_entryGroupID)
    {
        destroyCDKEntry(l_entryGroupID);
        l_entryGroupID = NULL;
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
void Umessage(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data)
{
    if (NULL == win)
    {
        UmessageDestory();
        return;
    }
    
    if (NULL == l_cdkscreen)
    {

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

