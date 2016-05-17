#include "UmonitorApp.h"
#include "Umonitor.h"
#include "Bmco/NumberParser.h"
#include "Bmco/FileStream.h"

#include <iostream>

namespace BM35 {
    void UmonitorApp::initialize(Application& self)
    {
        //loadConfiguration(); // load default configuration files, if present
        if (false == m_bFile)
        {
            loadConfiguration(config().getString("application.dir")+"../etc/"+
            config().getString("application.name")+".ini"); 
			loadConfiguration(config().getString("application.dir")+"../etc/BolPublic.ini");
        }
		
		std::string bolName = config().getString("info.bol_name");
		std::string controlPath = config().getString("memory.control.path");
		std::string controlName = config().getString("memory.control.name");		
        
        Application::initialize(self);
        // add your own initialization code here
    }
    
    void UmonitorApp::uninitialize()
    {
        // add your own uninitialization code here
        Application::uninitialize();
    }
    
    void UmonitorApp::reinitialize(Application& self)
    {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }
    
    void UmonitorApp::defineOptions(OptionSet& options)
    {
        Application::defineOptions(options);

        options.addOption(
            Option("help", "h", "display help information on command line arguments")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("cfg", "x", "define a menu configuration data from a xml file")
                .required(false)
                .repeatable(false)
                .argument("value")
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("file", "f", "load configuration data from a file")
                .required(false)
                .repeatable(false)
                .argument("file")
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("bind", "b", "bind option value to test.property")
                .required(false)
                .repeatable(false)
                .argument("value")
                .binding("test.property"));

        options.addOption(
            Option("Client", "C", "Client-Server ")
                .required(false)
                .repeatable(false)
                .argument("value")
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("omc", "M", "omc call")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("stopresume", "s", "stop and resume")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));

        options.addOption(
            Option("record", "r", "record info")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<UmonitorApp>(this, &UmonitorApp::handleOption)));
        
    }

    void UmonitorApp::handleOption(const std::string& name, const std::string& value)
    {
        if ("help" == name)
        {
            _helpRequested = true;
            displayHelp();
            stopOptionsProcessing();
            exit(1);
        }
        else if ("Client" == name)
        {
            m_Cilent = true;
            m_ipHostStr = value;
        }
        else if ("cfg" == name)
        {
            m_strXmlCfgFile = value;

            try
            {
                Bmco::FileIOS fo(std::ios::in);
                fo.open(value,std::ios::in);
                fo.close();
            }
            catch(Bmco::FileException &e)
            {
                printf("打开文件失败[%s]\n", e.displayText().c_str());
                exit(1);
            }
            catch(...)
            {
                puts("打开文件失败");
                exit(1);
            }
        }
        else if ("file" == name)
        {
            m_bFile = true;

            try
            {
                Bmco::FileIOS fo(std::ios::in);
                fo.open(value,std::ios::in);
                fo.close();
            }
            catch(Bmco::FileException &e)
            {
                printf("打开文件失败[%s]\n", e.displayText().c_str());
                exit(1);
            }
            catch(...)
            {
                puts("打开文件失败");
                exit(1);
            }

            loadConfiguration(value);
        }
        else if ("omc" == name)
        {
            m_bOMC = true;
        }
        else if ("stopresume" == name)
        {
            m_bStopResume = true;
        }
        else if ("record" == name)
        {
            m_bRecord = true;
        }
        else
        {
        }
    }

    void UmonitorApp::displayHelp()
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        //helpFormatter.setHeader("A sample application that demonstrates some of the features of the Poco::Util::Application class.");
        helpFormatter.format(std::cout);
    }

    void UmonitorApp::recordUserInfo()
    {
        int num = config().getInt("application.userNum");
        std::string username;
        std::string password;
        std::string authority;
        UserInfoAdmin tempVec;
        uia.clear();
        for (int i=1;i<=num;i++)
        {
            username = Bmco::format("application.username%?d", i);
            password = Bmco::format("application.password%?d", i);
            authority = Bmco::format("application.authority%?d", i);
            tempVec.username = config().getString(username.c_str());
            tempVec.password = config().getString(password.c_str());
            tempVec.authority = (UserLimit)config().getInt(authority.c_str());
            uia.push_back(tempVec);
        }
    }

    int UmonitorApp::main(const std::vector<std::string>& args)
    {
        recordUserInfo();
        Bmco::Int32 _argc = Bmco::NumberParser::parse(config().getString("application.argc"));

        /*if (true==m_bOMC && true==m_bStopResume)//omc获取停复机工单情况
        {
            g_pApp = this;
            GetStopResumeInfoForOMC();
            return Application::EXIT_OK;
        }
        else if (true==m_bOMC && true==m_bRecord)//omc获取话单处理情况
        {
            g_pApp = this;
            GetRecordInfoForOMC();
            return Application::EXIT_OK;
        }
        else
        {
        }*/
        
        // 交互式命令
        if (!m_Cilent)
        {
            UmonitorMain(args, this);
        }
        // 命令行式命令
        else
        {
            std::string terStr = "ssh " + m_ipHostStr;
            std::string prepareStr = terStr + " " + config().getString("application.remote_path") + "remote_bolmonitor.sh -a";
            FILE *fp1 = NULL;
            
            if (NULL == (fp1 = popen(prepareStr.c_str(), "r")))
            {
                pclose(fp1);
                return Application::EXIT_DATAERR;
            }
            pclose(fp1);
            execlp("ssh", "ssh", m_ipHostStr.c_str(), NULL);
        }
        /*if (!_helpRequested)
        {
            logger().information("Arguments to main():");
            for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it)
            {
                logger().information(*it);
            }
            logger().information("Application properties:");
            //printProperties("");
        }*/
        return Application::EXIT_OK;
    }
};

BMCO_APP_MAIN(BM35::UmonitorApp)
