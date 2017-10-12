
#include "signal_setter.h"

#include <sys/types.h>
#if defined (__unix__)
#include <unistd.h>
#endif

#include "debug.hpp"
#include <signal.h>
#include <errno.h>
#include <debug.hpp>

static bool register_sig_handler(int sig, signal_handler handler)
{
#if defined (__unix__)
	struct sigaction action;

	action.sa_handler = handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags |= SA_RESTART; 

	if( -1 == sigaction(sig, &action, NULL) )
	{
        LOG_ERROR_MSG(LOG_APP_ON, "Error setting %d signal handlers %s\n", sig, strerror(errno));
		return false;
	}
#endif
	return true;
}

bool set_signal_handlers(signal_handler handler)
{	
	if ( !handler )
		return false;

	static bool is_init = false;

	do 
	{
		if( is_init )
			break;
	
		const int sig_codes[] = 
		{
			SIGINT,
#if defined (__unix__)
			SIGTTIN,
#endif
			SIGTERM
		};
		
		bool is_registered = false;
		for(unsigned int i = 0; i < sizeof(sig_codes)/sizeof(sig_codes[0]); ++i)
		{
			is_registered = register_sig_handler( sig_codes[i], handler );
			if( !is_registered )
				break;
		}

		if( !is_registered )
			break;

		is_init = true;

	} while (false);

	return is_init;
}
