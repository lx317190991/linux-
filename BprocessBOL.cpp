#include "BprocessBOL.h"

#include "BolCommon/ControlRegionOp.h"
#include "BolCommon/MetaBpcbInfo.h"
#include "Bmco/DateTime.h"
#include "Bmco/DateTimeFormatter.h"
#include "Bmco/DateTimeFormat.h"

using Bmco::DateTimeFormatter;
using Bmco::DateTimeFormat;
using Bmco::DateTime;
using Bmco::Logger;

#include <string>
#include <vector>

using namespace std ;
using namespace Bmco;

//���������õ���ncurses-cdk�����
static CDKSCREEN *l_cdkscreen    = 0;//cdk��Ļ
static CDKSELECTION *l_selection = 0;//cdk��ѡ�б�ؼ�
static CDKENTRY *l_entryFlowID   = 0;//flowid������ؼ�
static CDKENTRY *l_entryBpcbID    = 0;//bpcbid������ؼ�
static CDKDIALOG *l_dialog       = 0;//����ѡ��Ի���ؼ�
//�������ϵ�����ָ��
static char **l_item             = 0;//��ѡ�б�����ָ��
static UserPtr *l_uptr           = 0;//��ѡ�б����ͷָ����鳤�ȵĽṹ��ָ��

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/*struct CreatDialog
{
    CDKSCREEN *cdkscreen    = 0;//cdk��Ļ
    CDKSELECTION *selection = 0;//cdk��ѡ�б�ؼ�
    CDKENTRY *entryFlowID   = 0;//flowid������ؼ�
    CDKENTRY *entryBpcbID    = 0;//bpcbid������ؼ�
    CDKDIALOG *dialog       = 0;//����ѡ��Ի���ؼ�
};
*/
namespace BM35{
static std::vector<MetaBpcbInfo> l_vecBpcbInfo;

//��MetaBpcbInfo�ṹ����Ϣ�Ա��浽�ַ����з��أ�������ڴ���ConvertDataToString�����ͷ�
static char *BpcbInfoToString(const MetaBpcbInfo &mbi, const int *separator)
{
    
    Bmco::UInt32 i = 0;
    Bmco::UInt32 len = 0;//selectionһ�еĳ���
    Bmco::UInt32 maxItemLen = 0;//selectionһ�������������󳤶�

    //MetaBpcbInfoOp* bpcbPtr = NULL;
    //std::vector<MetaBpcbInfo> vecBpcbInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOper(new ControlRegionOp(ctlAttrPtr, writer_t));
    MetaProgramDefOp *pMetaProgramDefOp = NULL;
    
    if (ctlOper->initialize())
    {
        pMetaProgramDefOp = dynamic_cast<MetaProgramDefOp*>(ctlOper->getObjectPtr(MetaProgramDefOp::getObjName()));
    }
    else
    {
        return NULL;
    }
    
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

    snprintf(item, separator[i], "%ld", mbi.m_iBpcbID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%d", mbi.m_iSysPID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%d", mbi.m_iProgramID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

	Bmco::Timestamp now;
    MetaProgramDef::Ptr ptr = new MetaProgramDef(mbi.m_iProgramID,0,0,0,0,0,0,"","","","",now,0);
    pMetaProgramDefOp->queryProgramDefByID(ptr);
    snprintf(item, separator[i], "%s", ptr->ExeName.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    switch(mbi.m_iStatus)
    {
       case ST_INIT:
           strncpy(item,"INIT", separator[i]);break;
       case ST_READY:
           strncpy(item,"READY", separator[i]);break;
       case ST_RUNNING:
           strncpy(item,"RUNNING", separator[i]);break;
       case ST_HOLDING:
           strncpy(item,"HOLDING", separator[i]);break;
       case ST_MAITENANCE:
           strncpy(item,"MAITENANCE", separator[i]);break;
       case ST_ABORT:
           strncpy(item,"ABORT", separator[i]); break;
       case ST_STOP:
           strncpy(item,"STOP", separator[i]); break;
       default:
           strncpy(item,"unknown", separator[i]);
    }
    //sprintf(item, "%d", mbi.m_iStatus);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    /*snprintf(item, separator[i], "%d", mbi.m_iFDLimitation);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%d", mbi.m_iFDInUsing);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;
    */

    snprintf(item, separator[i], "%d", mbi.m_iCPU);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%d", mbi.m_iRAM);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%u", mbi.m_iFlowID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item, separator[i], "%d", mbi.m_iInstanceID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    //snprintf(item, separator[i], "%d", mbi.m_iTaskSource);
    //memcpy(str+nextItem, item, strlen(item));
    //nextItem += separator[i];
    //i++;

    snprintf(item, separator[i], "%d", mbi.m_iSourceID);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    LocalDateTime dt(mbi.m_tThisTimeStartTime);
    std::string st = DateTimeFormatter::format(dt, DateTimeFormat::SORTABLE_FORMAT);
    snprintf(item, separator[i], "%s", st.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    dt = mbi.m_tLastTimeStartTime;
    st = DateTimeFormatter::format(dt, DateTimeFormat::SORTABLE_FORMAT);
    snprintf(item, separator[i], "%s", st.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    return str;
}

//��vector�е����ݰ���separator�еķָ���ת�����ַ�����������item�У�����item�е�������
static int ConvertDataToString(char ***item, const std::vector<MetaBpcbInfo> &vecInfo, const int *separator)
{
    int size = vecInfo.size();

    if (0 >= size)
    {
         return 0;
    }
    
    *item = (char **)malloc((size+1) * sizeof(*(*item)));

    int i=0, count=0;
    for (; i<size; i++)
    {
        char *p = BpcbInfoToString(vecInfo[i], separator);
        if (NULL != p)
        {
            (*item)[count] = p;
            count++;
        }
    }
    
    (*item)[count] = 0;

    return count;
}

//����:���ڴ����в�ѯ���ݣ�������ַ���������ʽ������item��
//���:separator��ʾ�ָ������飬flowid��pbcbid��ʾ��ѯ����
//����:item�����ѯ���
//����:��ѯ������������
static int LoadData(char ***item, const int *separator, int flowid, int pbcbid)
{
    bool ret = true;
    int count = 0;

    MetaBpcbInfoOp* bpcbPtr = NULL;
    std::vector<MetaBpcbInfo> vecBpcbInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));

	std::string bolName = g_pApp->config().getString("info.bol_name");
	std::string controlPath = g_pApp->config().getString("memory.control.path");
	std::string controlName = g_pApp->config().getString("memory.control.name");
    
    if (ctlOptPtr->initialize())
    {
        bpcbPtr = dynamic_cast<MetaBpcbInfoOp*>(ctlOptPtr->getObjectPtr(MetaBpcbInfoOp::getObjName()));
    }
    else
    {
        return count;
    }

    if(false == bpcbPtr->getAllBpcbInfo(vecBpcbInfo))
    {
        g_pApp->logger().information("Failed to execute getAllBpcbInfo on MetaShmBpcbInfoTable");
        return count;;
    }

    l_vecBpcbInfo.clear();
    
    for (int i=0; i<vecBpcbInfo.size(); i++)
    {
        if (-1!=flowid)
        {
            if (flowid != vecBpcbInfo[i].m_iFlowID)
            {
                continue;
            }
        }

        if (-1!=pbcbid)
        {
            if (pbcbid == vecBpcbInfo[i].m_iBpcbID)
            {
                continue;
            }
        }

        if (ST_INIT==vecBpcbInfo[i].m_iStatus || ST_ABORT==vecBpcbInfo[i].m_iStatus || ST_STOP==vecBpcbInfo[i].m_iStatus)
        {
            continue;
        }

        l_vecBpcbInfo.push_back(vecBpcbInfo[i]);
    }
    
    count = ConvertDataToString(item, l_vecBpcbInfo, separator);
    
    return count;
}

//ָֹͣ��bpcbID�Ľ���
static bool StopProcessByBpcbID(int bpcbID)
{
    MetaBpcbInfoOp* bpcbPtr = NULL;
    std::vector<MetaBpcbInfo> vecBpcbInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    MetaRegularProcessTaskOp* regularProcessPtr = dynamic_cast<MetaRegularProcessTaskOp*>(ctlOptPtr->getObjectPtr(MetaRegularProcessTaskOp::getObjName()));

    if (ctlOptPtr->initialize())
    {
        bpcbPtr = dynamic_cast<MetaBpcbInfoOp*>(ctlOptPtr->getObjectPtr(MetaBpcbInfoOp::getObjName()));
    }
    else
    {
        return false;
    }


    MetaBpcbInfo::Ptr ptr = new MetaBpcbInfo(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    if (!bpcbPtr->getBpcbInfoByBpcbID(bpcbID,ptr))
    {
        g_pApp->logger().information("Failed to execute getBpcbInfoByBpcbID on MetaShmBpcbInfoTable");
        return false;
    }

    bool isHere = false; //�����Ƿ�����
    if (!bpcbPtr->checkProcessAliveByBpcbID(bpcbID,isHere))
    {
        g_pApp->logger().information("Failed to execute checkProcessAliveByBpcbID on MetaShmBpcbInfoTable");
        return false;
    }

    if(false == isHere)
    {
        char s[64];
        sprintf(s, "����BpcbID[%d]:�ó��򲻴���,����Ҫֹͣ!", bpcbID);
        g_pApp->logger().information(std::string(s));
        return false;
    }
    else
    {
        Timestamp now;
        MetaRegularProcessTask::Ptr payload(new MetaRegularProcessTask(1, ptr->m_iProgramID, ptr->m_iInstanceID, 
            ptr->m_iFlowID, false, now, false, 0, "", false));
        //֪ͨ1#ȥֹͣ����
        if (!regularProcessPtr->Insert(payload))
        {
            g_pApp->logger().information("Failed to execute updateIsValidByBpcbID on MetaShmRegularProcessTaskTable");
            //std::cout<<Bmco::format("����ID[%?d]:����ֹͣʧ��",bpcbID)<<std::endl;
            return false;
        }
    }

    return true;
}

//����ָ��bpcbID�Ľ���
static bool StartProcessByBpcbID(int bpcbID)
{
    MetaBpcbInfoOp* bpcbPtr = NULL;
    std::vector<MetaBpcbInfo> vecBpcbInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    MetaRegularProcessTaskOp* regularProcessPtr = dynamic_cast<MetaRegularProcessTaskOp*>(ctlOptPtr->getObjectPtr(MetaRegularProcessTaskOp::getObjName()));
    
    if (ctlOptPtr->initialize())
    {
        bpcbPtr = dynamic_cast<MetaBpcbInfoOp*>(ctlOptPtr->getObjectPtr(MetaBpcbInfoOp::getObjName()));
    }
    else
    {
        return false;
    }


    MetaBpcbInfo::Ptr ptr = new MetaBpcbInfo(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    if (!bpcbPtr->getBpcbInfoByBpcbID(bpcbID,ptr))
    {
        g_pApp->logger().information("Failed to execute getBpcbInfoByBpcbID on MetaShmBpcbInfoTable");
        return false;
    }

    bool isHere = false; //�����Ƿ�����
    if (!bpcbPtr->checkProcessAliveByBpcbID(bpcbID,isHere))
    {
        g_pApp->logger().information("Failed to execute checkProcessAliveByBpcbID on MetaShmBpcbInfoTable");
        return false;
    }

    if(true == isHere)
    {
        char s[64];
        sprintf(s, "����BpcbID[%d]:�ó����Ѿ�����,����Ҫ������!", bpcbID);
        g_pApp->logger().information(std::string(s));
        return false;
    }
    else
    {
        Timestamp now;
        MetaRegularProcessTask::Ptr payload(new MetaRegularProcessTask(1, ptr->m_iProgramID, ptr->m_iInstanceID, 
            ptr->m_iFlowID, true, now, false, 0, "", false));
        //֪ͨ1#ȥ��������
        if (!regularProcessPtr->Insert(payload))
        {
            g_pApp->logger().information("Failed to execute updateIsValidByBpcbID on MetaShmRegularProcessTaskTable");
            return false;
        }
    }

    return true;
}
}

//��ʾ��ǰ���浽��Ļ��
static void show()
{
    if (NULL != l_entryFlowID) drawCDKEntry(l_entryFlowID, 0);
    if (NULL != l_entryBpcbID) drawCDKEntry(l_entryBpcbID, 0);
    if (NULL != l_selection) drawCDKSelection(l_selection, 0);
}

//���û�����Ļ�ϵĲ������󴫵ݸ�BM35::LoadData������ȥ��ȡ�µ����ݣ�û�����ݷ���false
//��reload���õĺ�������ռ䣬�ڵ��´ε���reloadʱ���ͷ���Щ�ռ�
//��selection�������Ƿ���false������true
static bool reload()
{
    char tmp[64] = {0};
    int iFlowID, iBpcbID;
    bool ret = true;

    //��ȡFlowID��ֵ
    strcpy(tmp, getCDKEntryValue(l_entryFlowID));
    
    if (0 == strlen(tmp))
    {
        iFlowID = -1;
    }
    else
    {
        iFlowID = atoi(tmp);
    }

    //��ȡBpcbID��ֵ
    strcpy(tmp, getCDKEntryValue(l_entryBpcbID));
    
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
    int index = getCDKSelectionCurrent(l_selection);
    reload();
    setCDKSelectionCurrent(l_selection, index);
    refresh();
    return (TRUE);
}

//֧�ֶ����ݵ�ȫѡ����ȫѡ
int ChoiceAll(EObjectType cdktype GCC_UNUSED,
                             void *object GCC_UNUSED, void *clientData, chtype key)
{
    CDKSELECTION *selt = (CDKSELECTION *)object;
    int listSize = getCDKSelectionItems(selt, NULL);//����item������
    bool bChoiceAll = TRUE;//�Ƿ��Ѿ�ȫѡ
    
    for (int i=0; i<listSize; i++)
    {
        if (selt->selections[i] == 0)
        {
            bChoiceAll = FALSE;
            break;
        }
    }

    int *choices = (int *)malloc(sizeof(int)*listSize);
    
    if (TRUE == bChoiceAll)
    {
        for(int j=0; j<listSize; j++)
        {
            choices[j] = 0;
        }
    }
    else
    {
        for(int j=0; j<listSize; j++)
        {
            choices[j] = 1;
        }
    }
    
    setCDKSelectionChoices(selt,choices);
    drawCDKObject(selt,TRUE);
    free(choices);
    
    return (TRUE);
}

//֧�ֶ����ݵĲ���ѡ��
int ChoicesPart(EObjectType cdktype GCC_UNUSED,
                             void *object GCC_UNUSED, void *clientData, chtype key)
{
    CDKSELECTION *selt = (CDKSELECTION *)object;
    int    currentItem = getCDKSelectionCurrent(selt);//��ǰѡ�����

    while (0 <= currentItem)
    {
        if (1 == getCDKSelectionChoice(selt, currentItem))
        {
            break;
        }
        
        setCDKSelectionChoice(selt, currentItem, 1);
        currentItem--;
    }
    
    drawCDKObject(selt,TRUE);
    
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

//�ڵ�һ�����뱾�����ʱ��������ʼ���������Widget
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
   
    if (NULL != l_entryFlowID)
    {
        destroyCDKEntry(l_entryFlowID);
        l_entryFlowID = NULL;
    }

    l_entryFlowID = newCDKEntry(l_cdkscreen,
                                1,
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
    
    if (NULL != l_entryBpcbID)
    {
        destroyCDKEntry(l_entryBpcbID);
        l_entryBpcbID = NULL;
    }

    l_entryBpcbID = newCDKEntry(l_cdkscreen,
                                30,
                                4,
                                NULL,
                                "BpcbID: ",
                                A_UNDERLINE | COLOR_PAIR(56),
                                '_',
                                vINT,
                                16, 0, 64,
                                FALSE,
                                FALSE);

    if (!l_entryBpcbID) {return false;}

    const char *message[4];
    message[0] = "<C></U>choose Command Interface";
    message[1] = "<C>1.Pick the command you wish to run and press enter!";
    message[2] = "<C>2.Press any key to quit from realtime refresh!     ";
    const char *buttons[] =
    {"</B/24>1.ˢ��<!B!24>", "</B/24>2.ʵʱˢ��<!B!24>", "</B/24>3.��������<!B!24>",
     "</B/24>4.ֹͣ����<!B!24>","</B/16>q.�˳�<!B!16>"};
        
    l_dialog = newCDKDialog(l_cdkscreen,
                            CENTER,
                            CENTER,
                            (CDK_CSTRING2)message, sizeof(message)/sizeof(message[0]),
                            (CDK_CSTRING2)buttons, sizeof(buttons)/sizeof(buttons[0]),
                            COLOR_PAIR(2) | A_REVERSE,
                            TRUE,
                            TRUE,
                            FALSE);

    if (!l_dialog) {return false;}
    bindCDKObject (vSELECTION, l_selection, KEY_F(5), SelectionFresh, NULL);
    bindCDKObject (vSELECTION, l_selection, 'a', ChoiceAll, NULL);
    bindCDKObject (vSELECTION, l_selection, 's', ChoicesPart, NULL);

    if (NULL != l_item)
    {
        CDKfreeStrings(l_item);
        l_item = NULL;
    }
    
    return true;
}

//�������Ĺ������򲢿�ʼ�û�����
static void traverse()
{
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
bj:
    while (true)
    {
        activateCDKEntry(l_entryBpcbID, NULL);

        if (KEY_ENTER == l_entryBpcbID->obj.input)
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
                                int index = getCDKSelectionCurrent(l_selection);
                                if (!reload())
                                {
                                    goto bj;
                                }
                                setCDKSelectionCurrent(l_selection, index);
                                refresh();
                                break;
                            }
                            case 1://2.ʵʱˢ��
                            {
                                halfdelay(10);
                                eraseCDKDialog(l_dialog);
                                while(ERR == getch())
                                {
                                    int index = getCDKSelectionCurrent(l_selection);
                                    reload();
                                    setCDKSelectionCurrent(l_selection, index);
                                    refresh();
                                }
                                nocbreak();
                                cbreak();
                                break;
                            }
                            case 2://3.��������
                            {
                                bool bchoiced = false;
                                
                                for (int i=0; i<BM35::l_vecBpcbInfo.size(); i++)
                                {
                                    if (1 == getCDKSelectionChoice(l_selection, i))
                                    {
                                        bchoiced = true;
                                        BM35::StartProcessByBpcbID(BM35::l_vecBpcbInfo[i].m_iBpcbID);
                                    }
                                }

                                if (false == bchoiced)
                                {
                                    BM35::StartProcessByBpcbID(BM35::l_vecBpcbInfo[getCDKSelectionCurrent(l_selection)].m_iBpcbID);
                                }
                                
                                break;
                            }
                            case 3://4.����kill
                            {
                                bool bchoiced = false;
                                
                                for (int i=0; i<BM35::l_vecBpcbInfo.size(); i++)
                                {
                                    if (1 == getCDKSelectionChoice(l_selection, i))
                                    {
                                        bchoiced = true;
                                        BM35::StopProcessByBpcbID(BM35::l_vecBpcbInfo[i].m_iBpcbID);
                                    }
                                }

                                if (false == bchoiced)
                                {
                                    BM35::StopProcessByBpcbID(BM35::l_vecBpcbInfo[getCDKSelectionCurrent(l_selection)].m_iBpcbID);
                                }

                                break;
                            }
                            case 4://5.��������
                            {
                                /*WINDOW *pwin = newwin(18, 50, (LINES-18)/2, (COLS-50)/2);
                                box(pwin, 0, 0);
                                CDKSCREEN *pcdkwin = initCDKScreen(pwin);*/
                                break;
                            }
                            case 5://6.��־
                            {
                                break;
                            }
                            case 6://q.�˳�
                            {
                                break;
                            }
                            default:
                            {
                            }
                        }
                    }
                }
            }
            else
            {
                break;
            } 
        }
    }
}

//�ͷŽ���ռ���ڴ�
static void BprocessBOLDestory()
{
    if (NULL != l_selection)
    {
        destroyCDKSelection(l_selection);
        l_selection = NULL;
    }

    if (NULL != l_entryFlowID)
    {
        destroyCDKEntry(l_entryFlowID);
        l_entryFlowID = NULL;
    }

    if (NULL != l_entryBpcbID)
    {
        destroyCDKEntry(l_entryBpcbID);
        l_entryBpcbID = NULL;
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

//���̹���ӿں���
void BprocessBOL(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data)
{
    if (NULL == win)
    {
        BprocessBOLDestory();
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

