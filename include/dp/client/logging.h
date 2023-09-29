#ifndef LOGGING_H
#define LOGGING_H

#include <string>

/**
 * This function will start proxying any data sent to stdout/stderr to the
 * corresponding OS logging system. Only Android is affected by this function
 * so far.
 */
int start_logger(const std::string& _tag);

#endif
