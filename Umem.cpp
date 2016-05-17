#include "Umem.h"

#include "Bmco/Util/Application.h"
#include "BolCommon/ControlRegionOp.h"
#include "BolCommon/MetaChunkInfo.h"
#include "BolCommon/MetaChunkInfoOp.h"

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
static CDKENTRY *l_entryMemID    = 0; //id������ؼ�
static CDKENTRY *l_entryRegionID = 0; //regionid������ؼ�
static CDKDIALOG *l_dialog       = 0; //����ѡ��Ի���ؼ�
//�������ϵ�����ָ��
static char **l_item             = 0;//��ѡ�б�����ָ��
//static vector<BM35::MetaBpcbInfo> l_vecMemInfo;
static UserPtr *l_uptr           = 0;//��ѡ�б����ͷָ����鳤�ȵĽṹ��ָ��

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



namespace BM35{
static std::vector<MetaChunkInfo> l_vecMemInfo;
//��MetaBpcbInfo�ṹ����Ϣ�Ա��浽�ַ����з��أ�������ڴ���ConvertDataToString�����ͷ�
static char *ChunkInfoToString(const MetaChunkInfo &mbi, const int *separator)
{
    
    Bmco::UInt32 i = 0;
    Bmco::UInt32 len = 0;//selectionһ�еĳ���
    Bmco::UInt32 maxItemLen = 0;//selectionһ�������������󳤶�
    float usedRate = 0.0;
    float usedByte = 0.0;
    
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

    snprintf(item,separator[i], "%d", mbi.id);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%d", mbi.regionId);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%ld", mbi.bytes);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%s", mbi.name.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    mbi.isGrow ? (strncpy(item, "true",separator[i])) : (strncpy(item, "false",separator[i]));
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    mbi.isMappedFile ? (strncpy(item, "true",separator[i])) : (strncpy(item, "false",separator[i]));
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    mbi.isShrink ? (strncpy(item, "true",separator[i])) : (strncpy(item, "false",separator[i]));
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    mbi.isSnapShot ? (strncpy(item, "true",separator[i])) : (strncpy(item, "false",separator[i]));
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%s", mbi.owner.c_str());
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    /*if (mbi.isMappedFile)
    {
        ShmMappedFile::Ptr shmPtr;
        shmPtr = new ShmMappedFile(mbi.name.c_str(), SHM_OPEN_READ_WRIT);
        if (shmPtr->attach()) 
        {
            usedByte = mbi.bytes - shmPtr->getFreeBytes();
            bmco_trace_f3(g_pApp->logger(),"%s|%s|getFreeBytes = %f",
                std::string("0"),std::string(__FUNCTION__),shmPtr->getFreeBytes());
            usedRate = usedByte/mbi.bytes;
        }
    }
    else
    {
        ShmSegment::Ptr shmPtr;
        shmPtr = new ShmSegment(mbi.name.c_str(), SHM_OPEN_READ_WRIT);
        if (shmPtr->attach()) 
        {
            usedByte = mbi.bytes - shmPtr->getFreeBytes();
            usedRate = usedByte/mbi.bytes;
        }
    }*/

	usedByte = (float)mbi.usedBytes;
	usedRate = usedByte/mbi.bytes;

    snprintf(item,separator[i], "%0.2f", usedByte);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    snprintf(item,separator[i], "%0.5f%%", usedRate*100);
    memcpy(str+nextItem, item, strlen(item));
    nextItem += separator[i];
    i++;

    return str;
}

//��vector�е����ݰ���separator�еķָ���ת�����ַ�����������item�У�����item�е�������
static int ConvertDataToString(char ***item, const std::vector<MetaChunkInfo> &vecInfo, const int *separator)
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
        (*item)[i] = ChunkInfoToString(vecInfo[i], separator);
        count++;
    }
    
    (*item)[i] = 0;

    return count;
}

//����:���ڴ����в�ѯ���ݣ�������ַ���������ʽ������item��
//����:separator��ʾ�ָ������飬memid��regionid��ʾ��ѯ����
//���:item�����ѯ���
//����:��ѯ������������
static int LoadData(char ***item, const int *separator, int memid, int regionid)
{
    int count = 0;

    MetaChunkInfoOp* chunkPtr = NULL;
    std::vector<MetaChunkInfo> vecChunkInfo;

    ControlRegionParams::Ptr ctlAttrPtr(new ControlRegionParams());
    CtlAttrExtractor()(g_pApp->config(), ctlAttrPtr);
    ControlRegionOp::Ptr ctlOptPtr(new ControlRegionOp(ctlAttrPtr, writer_t));
    if (!ctlOptPtr->initialize())
    {
        return count;
    }

    chunkPtr = dynamic_cast<MetaChunkInfoOp*>(ctlOptPtr->getObjectPtr(MetaChunkInfoOp::getObjName()));

    if (false == chunkPtr->Query(vecChunkInfo))
    {
        g_pApp->logger().information("Failed to execute getAllChunkInfo on MetaShmChunkInfoTable");
        return count;
    }

    if ((-1 == memid) && (-1 == regionid))//������������
    {
        count = ConvertDataToString(item, vecChunkInfo, separator);
        l_vecMemInfo = vecChunkInfo;
    }
    else if ((-1!=memid) && (-1==regionid))
    {
        l_vecMemInfo.clear();
        for (int i=0; i<vecChunkInfo.size(); i++)
        {
            if (memid == vecChunkInfo[i].id)
            {
                l_vecMemInfo.push_back(vecChunkInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMemInfo, separator);
    }
    else if ((-1==memid) && (-1!=regionid))
    {
        l_vecMemInfo.clear();
        for (int i = 0; i < vecChunkInfo.size(); i++)
        {
            if (regionid == vecChunkInfo[i].regionId)
            {
                l_vecMemInfo.push_back(vecChunkInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMemInfo, separator);
    }
    else
    {
        l_vecMemInfo.clear();
        for (int i=0; i<vecChunkInfo.size(); i++)
        {
            if (memid == vecChunkInfo[i].regionId && regionid == vecChunkInfo[i].id)
            {
                l_vecMemInfo.push_back(vecChunkInfo[i]);
            }
        }
        count = ConvertDataToString(item, l_vecMemInfo, separator);
    }
    
    return count;
}
}

static void show()
{
    //bool ret = false;
    if (NULL != l_entryMemID) drawCDKEntry(l_entryMemID, 0);
    if (NULL != l_entryRegionID) drawCDKEntry(l_entryRegionID, 0);
    if (NULL != l_selection) drawCDKSelection(l_selection, 0);
    //if (NULL != l_cdkscreen) refreshCDKScreen(l_cdkscreen);
    //ret = true;
    //return ret;
}

//���û�����Ļ�ϵĲ������󴫵ݸ�BM35::LoadData������ȥ��ȡ�µ����ݣ�û�����ݷ���false
//��reload���õĺ�������ռ䣬�ڵ��´ε���reloadʱ���ͷ���Щ�ռ�
//��selection�������Ƿ���false������true
static bool reload()
{
    char tmp[64] = {0};
    int iMemID = 0;
    int iRegionID = 0;
    bool ret = true;
    
    strcpy(tmp, getCDKEntryValue(l_entryMemID));
    
    if (0 == strlen(tmp))
    {
        iMemID = -1;
    }
    else
    {
        iMemID = atoi(tmp);
    }
    
    strcpy(tmp, getCDKEntryValue(l_entryRegionID));
    
    if (0 == strlen(tmp))
    {
        iRegionID = -1;
    }
    else
    {
        iRegionID = atoi(tmp);
    }
    
    int count = BM35::LoadData(&l_item, l_uptr->separator, iMemID, iRegionID);

    if (NULL != l_selection)
    {
        if (0 >= count)
        {
            ret = false;
            const char *tmp[]={" ", NULL};
            setCDKSelectionItems(l_selection, (CDK_CSTRING2)tmp, 1);
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
    bool ret = false;

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
   
    if (NULL != l_entryMemID)
    {
        destroyCDKEntry(l_entryMemID);
        l_entryMemID = NULL;
    }

    l_entryMemID = newCDKEntry(l_cdkscreen,
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

    if (!l_entryMemID) {return false;}
    
    if (NULL != l_entryRegionID)
    {
        destroyCDKEntry(l_entryRegionID);
        l_entryRegionID = NULL;
    }

    l_entryRegionID = newCDKEntry(l_cdkscreen,
                                30,
                                4,
                                NULL,
                                "RegionID: ",
                                A_UNDERLINE | COLOR_PAIR(56),
                                '_',
                                vINT,
                                16, 0, 64,
                                FALSE,
                                FALSE);

    if (!l_entryRegionID) {return false;}

    const char *message[4];
    message[0] = "<C></U>choose Command Interface";
    message[1] = "Pick the command you wish to run and press enter!";
    const char *buttons[] =
    {"</B/24>1.ˢ��<!B!24>","</B/24>3.checkpoint<!B!24>","</B/16>q.�˳�<!B!16>"};
        
    l_dialog = newCDKDialog(l_cdkscreen,
                            CENTER,
                            CENTER,
                            (CDK_CSTRING2)message, 2,
                            (CDK_CSTRING2)buttons, 1,
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
        activateCDKEntry(l_entryMemID, NULL);

        if (KEY_ENTER == l_entryMemID->obj.input)
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
        activateCDKEntry(l_entryRegionID, NULL);

        if (KEY_ENTER == l_entryRegionID->obj.input)
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
static void UmemDestory()
{
    if (NULL != l_selection)
    {
        destroyCDKSelection(l_selection);
        l_selection = NULL;
    }

    if (NULL != l_entryMemID)
    {
        destroyCDKEntry(l_entryMemID);
        l_entryMemID = NULL;
    }

    if (NULL != l_entryRegionID)
    {
        destroyCDKEntry(l_entryRegionID);
        l_entryRegionID = NULL;
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

// �����ڴ����ӿں���
void Umem(WINDOW *win, bool bloaddata, bool bshow, bool bactivate, void *data)
{
    if (NULL == win)
    {
        UmemDestory();
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

