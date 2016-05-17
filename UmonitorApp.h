#ifndef _UMONITORAPP_H_
#define _UMONITORAPP_H_
#include "Bmco/Util/Application.h"
#include "Bmco/Util/Option.h"
#include "Bmco/Util/OptionSet.h"
#include "Bmco/Util/HelpFormatter.h"
#include "Bmco/Util/AbstractConfiguration.h"
#include "Bmco/AutoPtr.h"

using Bmco::Util::Application;
using Bmco::Util::Option;
using Bmco::Util::OptionSet;
using Bmco::Util::HelpFormatter;
using Bmco::Util::AbstractConfiguration;
using Bmco::Util::OptionCallback;
using Bmco::AutoPtr;
using Bmco::Logger;

namespace BM35 {
enum UserLimit
{
    userRead,
    userWrite
};

typedef struct node
{
    std::string username;
    std::string password;
    UserLimit authority;
}UserInfoAdmin;

class UmonitorApp: public Application
    /// This sample demonstrates some of the features of the Util::Application class,
    /// such as configuration file handling and command line arguments processing.
    ///
    /// Try UmonitorApp --help (on Unix platforms) or UmonitorApp /help (elsewhere) for
    /// more information.
{
public:
    UmonitorApp(): _helpRequested(false),
        theLogger(Application::instance().logger())
    {
        m_Cilent = false;
        m_bFile = false;
        m_bOMC = false;
        m_bStopResume = false;
        m_bRecord = false;
        m_ipHostStr = "";
        m_strXmlCfgFile = "";
    }

    std::vector<UserInfoAdmin> uia;
    UserInfoAdmin currentUser;
    
protected:  
    void initialize(Application& self);
    void uninitialize();
    void reinitialize(Application& self);
    void defineOptions(OptionSet& options);
    void recordUserInfo();
    virtual void handleOption(const std::string& name, const std::string& value);
    //virtual void handleConfig(const std::string& name, const std::string& value);
    void displayHelp();
    int main(const std::vector<std::string>& args);

public:
    std::string m_strXmlCfgFile;
    
private:
    bool _helpRequested;
    Bmco::Logger&  theLogger;
    std::string m_ipHostStr;
    bool m_Cilent;
    bool m_bFile;
    bool m_bOMC;
    bool m_bStopResume;
    bool m_bRecord;
};
}
#endif
