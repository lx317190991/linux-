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

//本界面上用到的ncurses-cdk的组件
static CDKSCREEN *l_cdkscreen    = 0; //cdk屏幕
static CDKSELECTION *l_selection = 0; //cdk多选列表控件
static CDKENTRY *l_entryMemID    = 0; //id输入表单控件
static CDKENTRY *l_entryRegionID = 0; //regionid输入表单控件
static CDKDIALOG *l_dialog       = 0; //功能选项对话框控件
//本界面上的数据指针
static char **l_item             = 0;//多选列表数据指针
//static vector<BM35::MetaBpcbInfo> l_vecMemInfo;
static UserPtr *l_uptr           = 0;//多选列表标题和分割数组长度的结构体指针

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



namespace BM35{
static std::vector<MetaChunkInfo> l_vecMemInfo;
//把MetaBpcbInfo结构体信息以保存到字符串中返回，申请的内存由ConvertDataToString函数释放
static char *ChunkInfoToString(const MetaChunkInfo &mbi, const int *separator)
{
    
    Bmco::UInt32 i = 0;
    Bmco::UInt32 len = 0;//selection一行的长度
    Bmco::UInt32 maxItemLen = 0;//selection一行中所有项的最大长度
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

//把vector中的数据按照separator中的分割宽度转换成字符串，保存在item中，返回item中的总项数
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

//功能:从内存或表中查询数据，结果以字符串数组形式保存在item中
//输入:separator表示分割宽度数组，memid和regionid表示查询条件
//输出:item保存查询结果
//返回:查询到的数据条数
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

    if ((-1 == memid) && (-1 == regionid))//载入所有数据
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

//把用户在屏幕上的操作请求传递给BM35::LoadData函数，去获取新的数据，没有数据返回false
//被reload调用的函数申请空间，在第下次调用reload时，释放这些空间
//当selection无内容是返回false，否则true
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
    {"</B/24>1.刷新<!B!24>","</B/24>3.checkpoint<!B!24>","</B/16>q.退出<!B!16>"};
        
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

// 共享内存管理接口函数
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

