#include <cassert>

#include <warningset.h>

#include "shstrng.h"

static const char* SharedStrings[] =
{
	"",
	"Cannot get extensions",
	"Cannot get icon of the object",
	"Cannot initialize socket library",
	"Cannot create socket",
	"Error writing to the socket",
	"Error reading from the socket",
	"Connection closed",
	"Cannot resolve the host: %s",
	"Cannot connect to host %s",
	"Cannot login to the RA server",
	"Error opening rgb file %s for reading",
	"File %s is not a supported RGB format",
	"Out of memory",
	"No object selected",
	"Error opening object %s",
	"Folder %s does not seem to contain an L-studio object.\nOpen anyway?",
	"Error creating extension of object %s named %s",
	"Cannot delete oofs root object",
	"Do you want to delete object %s?",
	"Do you want to delete object %s and its extensions?",
	"Error deleting object %s",
	"Object copy/paste is not supported by this connection",
	"Error occured while pasting object",
	"Error occured while pasting objects",
	"Select the objects to be pasted first",
	"Unable to locate object  at %s",
	"Cannot rename object %s",
	"Hide &extensions\tGrey (+)",
	"Show &extensions\tGrey (+)",
	"Hide &icon\tGrey (*)",
	"Show &icon\tGrey (*)",
	"Cannot open config file %s",
	"There are objects open from this browser.\nIf you proceed you will lose any changes. Proceed?",
	"Cannot find object: %s",
	"Cannot find the object of id %d referenced by this hyperobject",
	"Hypercopying is not supported by this connection",
	"Could not open .id file of object",
	"Cannot find the object of id %d",
	"Could not finish paste operation, because could not create destination directory",
	"Could not create file \"%s\"",
	"Cannot paste object as child of hyperobject",
	"Do you want to delete hyperobject %s?",
	"Do you want to delete hyperobject %s and its extensions?",
	"Could not create the new hyperobject",
	"Cannot locate target path for hyperobject %s",
	"Cannot paste hyperobject across browsers",
	"Cannot get UUID for object at \"%s\"",
	"Cannot create object \"%s\"",
	"Command failed or is not supported",

	""
};

const char* SharedStr::GetLibString(int id)
{
	assert(id>0);
	assert(id<SharedStr::strLastString);
	return SharedStrings[id];
}
