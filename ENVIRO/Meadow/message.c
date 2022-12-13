/*

When writing your own environmental program you must supply 
function void Message(const char* format, ...)
This function is used by the communication library to display
messages (error messages and other diagnostics).
If your environmental program is a console program just
compile this file and link your program with the object file.
If your environmental program is a GUI program you have to
supply your own Message function.

*/


#include <stdarg.h>
#include <stdio.h>


void Message(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
