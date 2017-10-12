#pragma once

typedef void (*signal_handler)(int sig);
bool set_signal_handlers(signal_handler handler);

