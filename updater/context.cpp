#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "context.h"
#include "build_timestamp.h"
#include "version_info.h"


namespace application {



context& context::operator=(const context& src)
{
	if(this != &src)
	{
		cardreader	= src.cardreader;
		barcode		= src.barcode;
		pidfile		= src.pidfile;
        gprs_device = src.gprs_device;
	}

	return *this;
}

bool context::process_options(int argc, char** argv, std::string& out)
{
	bool rc = false;
	out.clear();

	namespace po = boost::program_options;

	std::string cardreader_value;
	std::string barcode_value;
	std::string pidfile_value;
    std::string gprs_value;

	do 
	{
		try
		{
			po::options_description generic_ops("Generic");
			generic_ops.add_options()
				("help,h"		, "show this help message")
				("version,v"	, "print version string")
				("timestamp,t"	, "return build timestamp")
				;

			po::options_description config_ops("Config");
			config_ops.add_options()
				("cardreader,c"	, po::value<std::string>(&cardreader_value),	"card reader device file name (/dev/cardreader)")
				("barcode,b"	, po::value<std::string>(&barcode_value),		"barcode reader device file name (/dev/barcode)")
                ("gprs,g"       , po::value<std::string>(&gprs_value),          "gprs device file name (/dev/gprs)")
				("pidfile,p"	, po::value<std::string>(&pidfile_value),		"pid file name");

			std::string module_name( boost::filesystem::basename(argv[0]) ); 
			std::ostringstream oss_usage;

			oss_usage << "Usage:" << std::endl;

			oss_usage << module_name << " [-h|--help]"      << std::endl;
			oss_usage << module_name << " [-v|--version]"   << std::endl;
			oss_usage << module_name << " [-t|--timestamp]" << std::endl;

			oss_usage << module_name;
			oss_usage << " [ [-c|--cardreader] <filename> ]";
			oss_usage << " [ [-b|--barcode] <filename> ]";
            oss_usage << " [ [-g|--gprs] <filename> ]";
			oss_usage << " [ [-p|--pidfile] <filename> ]";
			oss_usage << std::endl;
			
						
			po::options_description allowed_ops( oss_usage.str() );

			allowed_ops.add(generic_ops);
			allowed_ops.add(config_ops);

			po::variables_map vm;
			po::store(po::parse_command_line(argc, argv, allowed_ops, po::command_line_style::unix_style), vm);

			const bool x1 = vm.count("help");
			const bool x2 = vm.count("version");
			const bool x3 = vm.count("timestamp");
            const bool x4 = vm.count("cardreader") || vm.count("barcode") || vm.count("gprs") ||  vm.count("pidfile");

			char rc_parse = 0;
			if( x1 ) rc_parse |= 8;
			if( x2 ) rc_parse |= 4;
			if( x3 ) rc_parse |= 2;
			if( x4 ) rc_parse |= 1;
			
			const bool is_exp_of_2 = (0 == ((rc_parse - 1) & rc_parse));
			if( (0 == rc_parse) || !is_exp_of_2 )
			{
				std::ostringstream oss;
				oss << "Invalid args"  << std::endl;
				oss << oss_usage.str() << std::endl;
				out.append( oss.str() );
				break;
			}
			else if( 8 == rc_parse )
			{
				std::ostringstream oss;
				oss << allowed_ops;
				out = oss.str();
				rc = true;
				break;
			}
			else if( 4 == rc_parse )
			{
				out.append( application::version_info::to_string() );
				rc = true;
				break;
			}
			else if( 2 == rc_parse )
			{
				out.append( application::get_build_timestamp() );
				rc = true;
				break;
			}
			else if( 1 == rc_parse )
			{
				po::notify(vm);
			}
			
			//po::store(po::parse_command_line(argc, argv, allowed_ops, po::command_line_style::unix_style), vm);

			rc = true;
			break;
		}

		catch(const boost::program_options::error &ex)
		{
			out.append("Error: ");
			out.append(ex.what());
			break;
		}

		catch(...)
		{
			out.append("Unknown error.");
		}

	} while (false);

	if( rc )
	{
		cardreader  = cardreader_value;
		barcode		= barcode_value;
        gprs_device = gprs_value;
		pidfile		= pidfile_value;
	}

	return rc;
}

} // end namespace application
