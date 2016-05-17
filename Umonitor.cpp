#include "Umonitor.h"
#include "Bmco/Base64Decoder.h"
#include <sstream>

static MENU *g_pMenu = 0;
BM35::UmonitorApp *g_pApp = 0;

//#define COMMAND(NAME) {"#NAME", NAME}
//�������ͺ���ָ���ӳ��
struct FuncName
{
    //char funcName[128];
    const char *funcName;
    MenuFunc func;//�ص�����
    
    /*FuncName(char * fn, MenuFunc fc)
    {
        funcName[0] = 0;
        if (NULL != fn)strcpy(funcName, fn);
        func = fc;
    }*/
} NameToFunc[] = {
	                  {"BprocessBOL",BprocessBOL},
	                  {"Umessage",Umessage},
	                  {"Umem",Umem},
	                  {"UmessageGroup",UmessageGroup},
	                  {"UregionMem", UregionMem},
	                  {"UprogramDef", UprogramDef},
	                  {"UbpcbInfo", UbpcbInfo},
	                  {"UregularProcessTask", UregularProcessTask},
	                  {"UMQDef", UMQDef},
                   	  {"UMQRela", UMQRela},
                   	  {"UMQTopic", UMQTopic},
                   	  {"UMQService", UMQService}
                  };


//�ݹ���Ҳ˵��������������˵�
bool FindMenu(CMarkup *xml, MENU **upmenu, MENU **downmenu)
{
    char tmp[256];
    int len = 0;
    int itemcount = 0;
    int namelen = 0;
    
    //������ǰmenu��item������item���ֵ���󳤶�
    while(1)
    {
        if (xml->FindElem("item"))
        {
            itemcount++;
            xml->IntoElem();
            if (xml->FindElem("name"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                if (namelen < len){namelen = len;}
            }
            xml->OutOfElem();
        }
        else
        {
            xml->ResetMainPos();
            break;
        }
    }

    int i = 0;
    UserPtr *pup = NULL;
    ITEM **items = (ITEM **)calloc(itemcount+1, sizeof(ITEM *));
    
    while(1)
    {
        if (xml->FindElem("item"))
        {
            xml->IntoElem();

            char *name = NULL;
            if (xml->FindElem("name"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                name = new char[namelen+1];
                strncpy(name, tmp, len);
                name[len] = 0;
            }
            else
            {
                mvprintw(0, 0, "�Ҳ����˵�������");
                return false;
            }

            //�����˵���
            items[i] = new_item(" ", NULL);

            //���ò˵������֣���Ϊ������������ģ�����new_item����������Ե�������
            if (NULL != name)
            {
                items[i]->name.length = strlen(name);
                items[i]->name.str = name;
            }

            xml->ResetMainPos();
            char *desc = NULL;
            if (xml->FindElem("desc"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                desc = new char[len+1];
                strncpy(desc, tmp, len);
                desc[len] = 0;
            }

            //���ò˵�������
            if (NULL != desc)
            {
                items[i]->description.length = strlen(desc);
                items[i]->description.str = desc;
            }

            pup = new UserPtr;
            memset(pup, 0, sizeof(UserPtr));
            pup->upmenu = upmenu;

            xml->ResetMainPos();
            char *func = NULL;
            if (xml->FindElem("func"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                func = new char[len+1];
                strncpy(func, tmp, len);
                func[len] = 0;
                
                int num = sizeof(NameToFunc)/sizeof(FuncName);
                for(int j=0; j<num; j++)//������ӳ�䵽����ָ��
                {
                    if (0 == strcmp(func, NameToFunc[j].funcName))
                    {
                        pup->func = NameToFunc[j].func;
                    }
                }
            }

            xml->ResetMainPos();
            char *cmd = NULL;
            if(xml->FindElem("cmd"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                cmd = new char[len+1];
                strncpy(cmd, tmp, len);
                cmd[len] = 0;
                pup->cmd = cmd;
            }

            xml->ResetMainPos();
            char *title = NULL;
            if(xml->FindElem("title"))
            {
                strcpy(tmp, xml->GetData().c_str());
                len = strlen(tmp);
                title = new char[len+1];
                strncpy(title, tmp, len);
                title[len] = 0;
                pup->title = title;
            }

            xml->ResetMainPos();
            char intvstr[256] = {0};
            if(xml->FindElem("separator"))//�����ָ���
            {
                strcpy(intvstr, xml->GetData().c_str());
                len = strlen(intvstr);
                
                int i = 0;//��俪ͷ��' '��'|'
                while (' '==intvstr[i] || '|'==intvstr[i])
                {
                    intvstr[i] = ' ';
                    i++;
                }

                i = len-1;//����β��' '��'|'
                while (' '==intvstr[i] || '|'==intvstr[i])
                {
                    intvstr[i] = ' ';
                    i--;
                }

                int sepnumb = 0;//�ָ�ֵ�ĸ���
                i = 0;
                while (0 != intvstr[i])
                {
                    if ('|' == intvstr[i]) sepnumb++;
                    i++;
                }
                sepnumb++;

                i = 0;
                pup->separator = new int[sepnumb+1];
                (pup->separator)[sepnumb] = 0;
                char *p = strchr(intvstr, '|');
                char *pf = intvstr;
                while (NULL != p)//�������зָ�ֵ�ŵ����������У���0��β
                {
                    *p = 0;
                    (pup->separator)[i] = atoi(pf);
                    pf = p+1;
                    p = strchr(pf, '|');                    
                    i++;
                }
                (pup->separator)[i] = atoi(pf);
            }

            xml->ResetMainPos();
            if (xml->FindElem("menu"))
            {
                xml->IntoElem();
                if (false == FindMenu(xml, downmenu, &(pup->downmenu))){return false;}
                xml->OutOfElem();
            }

            set_item_userptr(items[i], pup);//�����û�ָ��
            xml->OutOfElem();
        }
        else
        {
            break;
        }
        
        i++;
    }

    items[i] = NULL;

    //�����˵�
    *downmenu = new_menu(items);
    //���ò˵�����
    set_menu_mark(*downmenu, "");//�˵����־����Ϊ��
    set_menu_format(*downmenu, 0, COLS);//���ò˵�����������
    menu_opts_off(*downmenu, O_SHOWDESC);//����ʾ����
    menu_opts_off(*downmenu, O_NONCYCLIC);//����ѭ������
    menu_opts_on(*downmenu, O_ROWMAJOR);
    menu_opts_on(*downmenu, O_SHOWMATCH);
    set_menu_fore(*downmenu, COLOR_PAIR(11) | A_REVERSE | A_STANDOUT);
    set_menu_back(*downmenu, COLOR_PAIR(24));
    set_menu_grey(*downmenu, COLOR_PAIR(48));
    (*downmenu)->spc_cols = 2;//������֮��ľ���

    return true;
}

//��xml�м��ز˵�
bool LoadMenu()
{
    CMarkup xml;
    std::string path;

    if (0 != g_pApp->m_strXmlCfgFile.length())
    {
        path = g_pApp->m_strXmlCfgFile;
    }
    else
    {
        path = g_pApp->config().getString("application.dir")+"../etc/minimonitor.xml";
    }
    
    if (false == xml.Load(path))
    {
        mvprintw(0, 0, "���������ļ�ʧ��");
        refresh();
        return false;
    }

    if (xml.FindElem("menu"))
    {
        xml.IntoElem();
        if (false == FindMenu(&xml, NULL, &g_pMenu)){return false;}
        xml.OutOfElem();
    }
    else
    {
        mvprintw(0, 0, "�Ҳ����˵�");
        return false;
    }
    
    return true;
}

//�ͷŲ˵��Լ������Ӳ˵����Լ����в˵���������Ľ���ռ�õ��ڴ�
void DestoryMenu(MENU *menu)
{
    ITEM **items = menu_items(menu);
    int i = 0;
    
    while (NULL != items[i])
    {
        if (NULL != items[i]->userptr)
        {
            if (NULL != ((UserPtr *)(items[i]->userptr))->downmenu)
            {
                DestoryMenu(((UserPtr *)(items[i]->userptr))->downmenu);
            }

            if (NULL != ((UserPtr *)(items[i]->userptr))->func)
            {
                //�ͷŲ˵���������Ľ���ռ�õ��ڴ�
                (((UserPtr *)(items[i]->userptr))->func)(NULL, false, false, false, NULL);
            }

            if (NULL != ((UserPtr *)(items[i]->userptr))->cmd)
            {
                delete ((UserPtr *)(items[i]->userptr))->cmd;
                ((UserPtr *)(items[i]->userptr))->cmd = NULL;
            }

            if (NULL != ((UserPtr *)(items[i]->userptr))->title)
            {
                delete ((UserPtr *)(items[i]->userptr))->title;
                ((UserPtr *)(items[i]->userptr))->title = NULL;
            }

            if (NULL != ((UserPtr *)(items[i]->userptr))->separator)
            {
                delete ((UserPtr *)(items[i]->userptr))->separator;
                ((UserPtr *)(items[i]->userptr))->separator = NULL;
            }
        }
        
        i++;
    }

    free_menu(menu);
}

// �û���½��Ȩ
bool CheckLoginInfo(const char* UserName, const char* Password)
{
    std::vector<BM35::UserInfoAdmin>::iterator it;
    std::string s;
    for (it = g_pApp->uia.begin();it != g_pApp->uia.end();it++)
    {   
        // ���������
        try
        {
            std::istringstream istr(it->password);
            Bmco::Base64Decoder decoder(istr);
            decoder >> s;
        }
        catch (...)
        {
            bmco_error_f2(g_pApp->logger(), "%s|%s|decode bad format",
                std::string("0"),std::string(__FUNCTION__));
            return false;
        }
        if (0 == strcmp(UserName, it->username.c_str()) 
            && (0 == strcmp(Password, s.c_str())))
        {
            g_pApp->currentUser.username = it->username;
            g_pApp->currentUser.password = it->password;
            g_pApp->currentUser.authority = it->authority;
            bmco_information_f3(g_pApp->logger(), "%s|%s|user[%s] login successfully!",
                std::string("0"),std::string(__FUNCTION__),std::string(UserName));
            return true;
        }
    }
    
    bmco_error_f3(g_pApp->logger(), "%s|%s|user[%s] login failed!",
        std::string("0"),std::string(__FUNCTION__),std::string(UserName));
    return false;
}

bool PopupLoginWin()
{
    //�߶ȣ���ȣ�Y���꣬X����
    int H=7+2, W=40, Y=(LINES-H)/2, X=(COLS-W)/2;
    if (COLS < W) W=COLS;
    if (LINES < H) H=LINES;
    WINDOW * setwin = newwin(H, W, Y, X);
    keypad(setwin, TRUE);
    box(setwin, 0, 0);
    //��ӡ������Ϣ
    wattron(setwin, COLOR_PAIR(56) | A_UNDERLINE);
    const char t[] = "Enter UserName And PassWord";
    mvwprintw(setwin, 1, (W-strlen(t))/2, t);
    wattroff(setwin, COLOR_PAIR(56) | A_UNDERLINE);
    //��ʾ����
    wrefresh(setwin);
    //��ʼ��cdk����
    CDKSCREEN *paramscreen = initCDKScreen(setwin);

    //int itmeMaxLen = 0;//�ֶ��б����������󳤶�
    const int MaxUserNamePasswordLen = 256;
    
    CDKENTRY *entryUserName = newCDKEntry(paramscreen,
                                          X+3,
                                          Y+3,
                                          NULL,
                                          "UserName:",
                                          A_UNDERLINE | COLOR_PAIR(56),
                                          '_',
                                          vMIXED,
                                          25, 0, MaxUserNamePasswordLen,
                                          FALSE,
                                          FALSE);

    drawCDKEntry(entryUserName, 0);

    CDKENTRY *entryPassword = newCDKEntry(paramscreen,
                                          X+3,
                                          Y+4,
                                          NULL,
                                          "PassWord:",
                                          A_INVIS | COLOR_PAIR(56),
                                          '_',
                                          vMIXED,
                                          25, 0, MaxUserNamePasswordLen,
                                          FALSE,
                                          FALSE);
        
    drawCDKEntry(entryPassword, 0);
    
    const char *buttons[3] =
    {
        "  OK  ",
        "Cancel",
        NULL
    };

    CDKBUTTONBOX *btnbox = newCDKButtonbox(paramscreen,
                                           CENTER, BOTTOM, 1, W/2,
                                           NULL, 1, 2,
                                           (CDK_CSTRING2)buttons, 2, 
                                           A_REVERSE, FALSE, FALSE);

    //��ʼ������
    drawCDKButtonbox(btnbox, 0);
    box(setwin, 0, 0);
    refreshCDKScreen(paramscreen);

    boolean functionKey;
    chtype input = 0;
    //int ret = -1;
    //int i = 0;
    bool bret = false;

//bj:
    
    //��ʼ�û�������ֱ���û���ESC������OK/Cancel�ϰ��س�������������
    while (TRUE)
    {
        //if (0 <= ret) break;

        activateCDKEntry(entryUserName, 0);
        activateCDKEntry(entryPassword, 0);
        drawCDKButtonbox(btnbox, ObjOf(btnbox)->box);

        while (TRUE)
        {
            input = (chtype)getchCDKObject(ObjOf(btnbox), &functionKey);

            if (KEY_TAB == input)
            {
                if (getCDKButtonboxCurrentButton(btnbox) == getCDKButtonboxButtonCount(btnbox)-1)
                {
                    setCDKButtonboxCurrentButton(btnbox, 0);
                    //i = 0;
                    break;
                    //goto bj;
                }
            }
            else if (KEY_ENTER == input)
            {
                if (0 == getCDKButtonboxCurrentButton(btnbox))//�û�ѡ��OK
                {
                    std::string msg;//��������

                    const char *pUserName = getCDKEntryValue(entryUserName);
                    const char *pPassword = getCDKEntryValue(entryPassword);

                    if (CheckLoginInfo(pUserName, pPassword))//��֤�ɹ�
                    {
                        const char *mesg[3] = {0};
                        mesg[0] = "<C>��֤�ɹ�,����";
                        char count[32] = {0};
                        strcpy(count, "<L>������");
                        mesg[1] = count;
                        
                        halfdelay(2);
                        int i=0;

                        while(i<6)
                        {
                            strcat(count, ".");
                            popupLabel(paramscreen, (CDK_CSTRING2) mesg, 2);
                            i++;
                        }
                        
                        nocbreak();
                        cbreak();

                        bret = true;
                        goto bj;
                    }
                    else//��֤ʧ��
                    {
                        const char *mesg[3] = {0};
                        mesg[0] = "<C>��֤��ʾ:";
                        mesg[1] = "<C>�û������������";
                        popupLabel(paramscreen, (CDK_CSTRING2) mesg, 2);
                        bret = false;
                        //goto bj;
                        break;
                    }
                }
                else//�û�ѡ��Cancel
                {
                    bret = false;
                    goto bj;
                }
            }
            else
            {
            }
            
            //Inject the character into the widget.
            injectCDKButtonbox(btnbox, input);
            //if (0 <= ret)
            //{
                //i = 0;
            //    break;
            //}
        }
    }

    //if (0 <= ret)//�û����˻س���
    //{
    //}
bj:
    //�ͷ���Ļ��Դ���ڴ�
    destroyCDKEntry(entryUserName);
    destroyCDKEntry(entryPassword);
    destroyCDKButtonbox(btnbox);
    destroyCDKScreen(paramscreen);
    werase(setwin);
    delwin(setwin);

    return bret;
}

int UmonitorMain(const std::vector<std::string>& args, BM35::UmonitorApp *pApp)
{
    if (NULL == g_pApp)
    {
        g_pApp = pApp;
    }
    else
    {
        return 0;
    }
    
    WINDOW *menu_win;
    
    //��ʼ��curses
    initscr();

    //����CDK��ɫϵͳ
    initCDKColor ();
    cbreak();//�����л���
    noecho();//��ֹ����
    keypad(stdscr, TRUE);

    //��¼��֤
    if (false == PopupLoginWin())
    {
        endwin();
        return 1;
    }
    
    //���ز˵�
    if (false == LoadMenu())
    {
        mvprintw(1, 0, "���ز˵�ʧ��");
        refresh();
        getch();
        endwin();
        exit(1);
    }

    const char tip[] = "�����л�-Tab  �����˵�/ȷ��-Enter  ˢ��-F5  �鿴-�������ҷ����  ��ݲ˵���-���ּ�/F1/q/PgUp/PgDn  ѡ��-space/a/s";
    attron(COLOR_PAIR(56));
    mvprintw(LINES-1, 0, tip);
    attroff(COLOR_PAIR(56));
    attron(COLOR_PAIR(8));
    mvprintw(3, 1,(g_pMenu->items)[0]->description.str);
    attroff(COLOR_PAIR(8));
    refresh();

    //������˵�������Ĵ���
    menu_win = newwin(3, COLS, 0, 0);
    keypad(menu_win, TRUE);
    //������ʾ����
    WINDOW *showwin = newwin(LINES-5, COLS, 4, 0);
    box(showwin, 0, 0);//��ӡ���ڱ߿���
    keypad(showwin, TRUE);
    wrefresh(showwin);
    
    set_menu_win(g_pMenu, menu_win);//�Ѳ˵����õ�����
    WINDOW *psw = derwin(menu_win, 0, 0, 1, 1);//�����Ӵ���
    set_menu_sub(g_pMenu, psw);//�Ѳ˵����õ����ڵ��Ӵ���
    post_menu(g_pMenu);//���Ͳ˵�
    wrefresh(menu_win);

    int mcount = item_count(g_pMenu);//�˵�������
    MENU *pMenu = g_pMenu;
    box(menu_win, 0, 0);//��ӡ���ڱ߿���

    MenuFunc funcurr0 = NULL;
    MenuFunc funcurr1 = NULL;
    int c;

    while(c = wgetch(menu_win))
    {
        attron(COLOR_PAIR(56));
        mvprintw(LINES-1, 0, tip);
        attroff(COLOR_PAIR(56));
    
        switch(c)
        {
            case KEY_F(1):
            {
                if (pMenu == g_pMenu)
                {
                    CDKSCREEN *cdkscreen = initCDKScreen(stdscr);
                    const char *message[2] = {0};
                    message[0] = "<C>�Ƿ���Ҫ�˳�����";
                    const char *buttons[] ={"</B/24>ȷ��<!B!24>","</B/24>ȡ��<!B!24>"};
                    CDKDIALOG *pdlg = newCDKDialog(cdkscreen,
                                                    CENTER,
                                                    CENTER,
                                                    (CDK_CSTRING2)message, 1,
                                                    (CDK_CSTRING2)buttons, 2,
                                                    COLOR_PAIR(2) | A_REVERSE,
                                                    TRUE,
                                                    TRUE,
                                                    FALSE);

                    int selection = activateCDKDialog(pdlg, 0);
                    eraseCDKDialog(pdlg);
                    destroyCDKDialog(pdlg);
                    destroyCDKScreen(cdkscreen);
                    
                    if (0 == selection)
                    {
                        goto end;
                    }
                    else
                    {
                        if (NULL != funcurr0) funcurr0(showwin, false, true, false, NULL);
                    }
                }
                else
                {
                    pMenu = *(((UserPtr *)item_userptr(current_item(pMenu)))->upmenu);
                    mcount = item_count(pMenu);//�˵�������
                    wmove(menu_win, 1, 1);
                    wclrtoeol(menu_win);
                    unpost_menu(pMenu);
                    set_menu_win(pMenu, menu_win);//�Ѳ˵����õ�����
                    set_menu_sub(pMenu, psw);//�Ѳ˵����õ����ڵ��Ӵ���
                    post_menu(pMenu);//���Ͳ˵�
                    box(menu_win, 0, 0);
                    wrefresh(menu_win);
                }
                break;
            }            
            case KEY_LEFT:
            {
                menu_driver(pMenu, REQ_LEFT_ITEM);
                break;
            }
            case KEY_RIGHT:
            {
                menu_driver(pMenu, REQ_RIGHT_ITEM);
                break;
            }
            case ' ':
                menu_driver(pMenu, REQ_TOGGLE_ITEM);
                break;
            case KEY_NPAGE:
            case KEY_PPAGE:
            {
                UserPtr *uptr = (UserPtr *)item_userptr(current_item(pMenu));
                if (NULL != funcurr0 && NULL != funcurr1)
                {
                    werase(showwin);
                    box(showwin, 0, 0);
                    MenuFunc t = funcurr1;
                    funcurr1 = funcurr0;
                    funcurr0 = t;
                    funcurr0(showwin, false, true, false, NULL);
                }
                break;
            }
            case '\t'://Tab����
            {
                if (NULL != funcurr0)
                {
                    funcurr0(showwin, false, true, true, NULL);
                }
                break;
            }
            case '\n'://ִ�в˵�����
            {
                if (mcount-1 == item_index(current_item(pMenu)))
                {
                    if (pMenu == g_pMenu)
                    {
                        CDKSCREEN *cdkscreen = initCDKScreen(stdscr);
                        const char *message[2] = {0};
                        message[0] = "<C>�Ƿ���Ҫ�˳�����";
                        const char *buttons[] ={"</B/24>ȷ��<!B!24>","</B/24>ȡ��<!B!24>"};
                        CDKDIALOG *pdlg = newCDKDialog(cdkscreen,
                                                        CENTER,
                                                        CENTER,
                                                        (CDK_CSTRING2)message, 1,
                                                        (CDK_CSTRING2)buttons, 2,
                                                        COLOR_PAIR(2) | A_REVERSE,
                                                        TRUE,
                                                        TRUE,
                                                        FALSE);

                        int selection = activateCDKDialog(pdlg, 0);
                        eraseCDKDialog(pdlg);
                        destroyCDKDialog(pdlg);
                        destroyCDKScreen(cdkscreen);
                        
                        if (0 == selection)
                        {
                            goto end;
                        }
                        else
                        {
                            if (NULL != funcurr0) funcurr0(showwin, false, true, false, NULL);
                            break;
                        }
                    }

                    if (mcount-1 == item_index(current_item(pMenu)))
                    {
                        pMenu = *(((UserPtr *)item_userptr(current_item(pMenu)))->upmenu);
                        mcount = item_count(pMenu);//�˵�������
                        wmove(menu_win, 1, 1);
                        wclrtoeol(menu_win);
                        unpost_menu(pMenu);
                        set_menu_win(pMenu, menu_win);//�Ѳ˵����õ�����
                        set_menu_sub(pMenu, psw);//�Ѳ˵����õ����ڵ��Ӵ���
                        post_menu(pMenu);//���Ͳ˵�
                        box(menu_win, 0, 0);
                        wrefresh(menu_win);
                    }
                }
                else
                {
                    UserPtr *uptr = (UserPtr *)item_userptr(current_item(pMenu));

                    if (NULL == uptr->downmenu)
                    {
                        if (NULL != uptr->cmd)
                        {
                            def_prog_mode();
                            endwin();
                            system(uptr->cmd);
                            reset_prog_mode();
                            refresh();
                            break;
                        }

                        if (NULL != uptr->func)//����xml���õĺ���
                        {
                            werase(showwin);
                            wborder(showwin, ' ',' ',' ',' ',' ',' ',' ',' ');
                            wrefresh(showwin);
                            (uptr->func)(showwin, true, true, false, uptr);
                            funcurr1 = funcurr0;
                            funcurr0 = uptr->func;
                            break;
                        }
                        else//û�к�������ʾ�û�
                        {
                            CDKSCREEN *cdkscreen = 0;
                            cdkscreen = initCDKScreen(showwin);
                            
                            const char *mesg[4] = {0};
                            mesg[0] = "<C>The item of menu that you selected is not attach function or command.";
                            mesg[1] = "";
                            mesg[2] = "<C>Press any key to continue.";
                            popupLabel(cdkscreen, (CDK_CSTRING2) mesg, 3);
                            
                            destroyCDKScreen(cdkscreen);
                            
                            if (NULL != funcurr0)
                            {
                                funcurr0(showwin, false, true, false, NULL);
                            }
                        }
                    }
                    else
                    {
                        pMenu = ((UserPtr *)item_userptr(current_item(pMenu)))->downmenu;
                        mcount = item_count(pMenu);//�˵�������
                        wmove(menu_win, 1, 1);
                        wclrtoeol(menu_win);
                        unpost_menu(pMenu);
                        set_menu_win(pMenu, menu_win);//�Ѳ˵����õ�����
                        set_menu_sub(pMenu, psw);//�Ѳ˵����õ����ڵ��Ӵ���
                        post_menu(pMenu);//���Ͳ˵�
                        box(menu_win, 0, 0);
                        wrefresh(menu_win);
                    }
                }
                break;
            }
            default:
            {
                if ('1' <= c && c <= '9')
                {
                    if (c <= (item_count(pMenu)+'0'))
                    {
                        set_current_item(pMenu, (menu_items(pMenu))[c-'0'-1]);
                        pos_menu_cursor(pMenu);
                    }
                }

                if ('q'==c || 'Q'==c)
                {
                    set_current_item(pMenu, (menu_items(pMenu))[item_count(pMenu)-1]);
                    pos_menu_cursor(pMenu);
                }
            }
        }
        box(menu_win, 0, 0);
        wmove(stdscr, 3, 0);
        clrtoeol();

        //��ӡ��ʾ��Ϣ
        if (g_pMenu == pMenu)
        {
            mvprintw(3, 1, item_description(current_item(pMenu)));
        }
        else
        {
            char t[1024] = {0};
            ITEM* pI = current_item(g_pMenu);
            strcpy(t, item_name(pI));
            //strcat(t, "-->");
            MENU *pM = g_pMenu;
            while (true)
            {
                pM = ((UserPtr *)item_userptr(pI))->downmenu;
                if (NULL == pM) break;
                strcat(t, "-->");
                pI = current_item(pM);
                strcat(t, item_description(pI));
            }

            mvprintw(3, 1, t);
        }
        
        refresh();
        wrefresh(menu_win);
    }

end:
    unpost_menu(pMenu);//Detach menu from screen
    DestoryMenu(g_pMenu);//�ͷ�ռ�õ��ڴ�
    endwin();
}
