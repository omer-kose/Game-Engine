#include "logger.h"
#include "asserts.h" // Need to include this to define the report_assertion_failure function. 
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h> // Allows us to work with variadic arguments

typedef struct logger_system_state
{
    b8 initialized;
} logger_system_state;

static logger_system_state* state_ptr;

b8 initialize_logging(u64* memory_requirement, void* state)
{
    *memory_requirement = sizeof(logger_system_state);
    if(state == 0)
    {
        return true;
    }

    state_ptr = state;
    state_ptr->initialized = true;

    // TODO: Create a log file
    return true;
}

void shutdown_logging(void* state)
{
    state_ptr = 0;
    // TODO: cleanup logging/write queued entries
}

/*
    ... is called variadic arguments. It works in the same way it works in printf.
*/
void log_output(log_level level, const char* message, ...)
{
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    const i32 msg_length = 32000;
    char out_message[msg_length];
    memset(out_message, 0, sizeof(out_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message); // Second argument is the last argument before the variadic argument
    vsnprintf(out_message, msg_length, message, arg_ptr); // Takes the buffer and variadic arguments, formats them and writes into the first argument out_message
    va_end(arg_ptr);

    char out_message2[msg_length];
    // Prepend the log_level identifier to the out_message
    sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

    // Platform-specific output.
    if(is_error)
    {
        platform_console_write_error(out_message2, level);
    }
    else
    {
        platform_console_write(out_message2, level);
    }

}


void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) 
{
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}