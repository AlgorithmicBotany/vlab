#include <cassert>

#include "libstrng.h"



static const char* FWstrings[] =
{
	"",
	"Error loading bitmap",
	"Error reading BMP file %s",
	"Error loading icon",
	"Unsupported or incorrect BMP file format",
	"Cannot access the clipboard",
	"Cannot allocate memory for clipboard",
	"Error",
	"Floating point number expected",
	"Integer expected",
	"Error creating synchronization object",
	"[LIBSTR] Cannot open file: %s",
	"Error reading file: %s",
	"Error writing to file: %s",
	"Unexpected end of file %s",
	"Line %d in the file %s is too long",
	"Cannot create font",
	"Error loading menu",
	"Timeout",
	"Error manipulating registry",
	"Cannot open semaphore %s",
	"Error creating thread",
	"Cannot change directory to %s",
	"[LIBSTR 2] Cannot open file: %s",
	"Message",
	"Please decide",
	"Error",
	"Cannot create window %s. System says: %d",
	"Cannot create temporary file"
};


const char* FWStr::GetLibString(int ix)
{ 
	assert(ix>0);
	assert(ix<FWStr::nmLastString);
	return FWstrings[ix];
}
