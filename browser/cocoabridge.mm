#include "cocoabridge.h"
#import <Cocoa/Cocoa.h>

void  CocoaBridge::setAllowsAutomaticWindowTabbing(bool flag)
 {
	[NSWindow setAllowsAutomaticWindowTabbing: flag];
 }
