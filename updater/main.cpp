#include <signal.h>

#if !defined (__unix__)
#include <process.h>
#endif

#include <QTextCodec>
#include <QApplication>

#include "debug.hpp"
#include "context.h"
#include "signal_setter.h"
#include "logger.hpp"
#include "Formatter.h"

namespace application {

namespace exit_codes
{
constexpr int SUCCESS               = 0;
constexpr int OPTIONS_FAILURE		= 1;
constexpr int UNHANDLED_EXCEPTION	= 2;
constexpr int ERROR_SAVE_PID		= 3;
constexpr int ERROR_SET_SIGNALS     = 4;
constexpr int ERROR_INITIALISE      = 5;
constexpr int ERROR_OPEN_DB         = 6;
};

} // end namespace application

static bool save_pid(const char* filename, int pid);
static void sig_handler(int);


class DestroyLogger
{
public:
    DestroyLogger(){}
    ~DestroyLogger()
    {
        //Utils::Logger::Destroy();
    }
};

int main(int argc, char *argv[])
{
    //Utils::Logger::Instance()->SetDirectory(L"/validator/logs");
    DestroyLogger destroyLogger;
    ////Utils::Logger::Instance()->SetImpotance();
    //Utils::Logger::Log(Utils::Severity_Error, Utils::Formatter("Starting..."));
    std::cerr<<"Starting..."<<std::endl;
	application::context options;

	std::string out;
	const bool is_options_valid = options.process_options(argc, argv, out);
    DEBUG_ASSERT(LOG_APP_ON, is_options_valid, "Error process program args");
	if( !out.empty() )
    {
		std::ostream* os = is_options_valid ? &std::cout : &std::cerr;
		(*os) << out << std::endl;
		return is_options_valid
            ? application::exit_codes::SUCCESS
            : application::exit_codes::OPTIONS_FAILURE;
    }

    if( !options.pidfile.empty() && !save_pid(options.pidfile.c_str(), getpid()) )
	{
        //LOG_ERROR_MSG(LOG_APP, "Error opening pidfile %s", options.pidfile.c_str());
		std::cerr << "Error opening pidfile " << std::endl;
        return application::exit_codes::ERROR_SAVE_PID;
	}

	if( !set_signal_handlers( sig_handler ) )
	{
        //LOG_ERROR_MSG(LOG_APP, "Error setup unix signal handlers on line %d", __LINE__);
        return application::exit_codes::ERROR_SET_SIGNALS;
	}
		
	// 2017-05-24 13:30
	// For correct UTF-8 strings interpretation.
	QTextCodec * codec = QTextCodec::codecForName("UTF-8");
    if(nullptr != codec)
	{
		//QTextCodec::setCodecForCStrings(codec);
		QTextCodec::setCodecForLocale(codec);
		//QTextCodec::setCodecForTr(codec);
	}

    QApplication a(argc, argv);
    std::cerr << logger()<< "gprs device=" << options.gprs_device <<std::endl;

    try
    {
//        Manager::Manager manager;

        a.setOverrideCursor(QCursor(Qt::BlankCursor));

        const int rc = a.exec();

        //LOG_DEBUG_MSG(LOG_APP, "Qt exit code = %d", rc);

        //Utils::Logger::Log(Utils::Severity_Info, Utils::Formatter("Finish application"));

        return rc;
    }
    catch (std::exception& ex)
    {
        //Utils::Logger::Log(Utils::Severity_Info, Utils::Formatter("ERROR: %s")%ex.what());
    }
    catch(...)
    {
        //Utils::Logger::Log(Utils::Severity_Info, Utils::Formatter("Unknown ERROR!!!"));
    }

    //Utils::Logger::Log(Utils::Severity_Info, Utils::Formatter("Abort application"));
    return application::exit_codes::UNHANDLED_EXCEPTION;
}

bool save_pid(const char* filename, int pid)
{
	std::fstream pidfile;
	pidfile.open(filename, std::fstream::out | std::fstream::trunc);
	if( !pidfile.good() )
		return false;

	pidfile << pid << std::endl;
	pidfile.close();
	return true;
}

static void sig_handler(int sig)
{
	if(SIGTERM == sig || SIGINT == sig
#if defined (__unix__)
		|| SIGTTIN == sig
#endif
		)
	{
		//2017-05-26 09:53
		//TODO: QApplication::exit(), QApplication::terminate()?
		//QApplication::closeAllWindows();
        //Utils::Logger::Log(Utils::Severity_Info, Utils::Formatter("if(SIGTERM == sig || SIGINT == sig || SIGTTIN == sig)"));

		QApplication::quit();
	}
}
