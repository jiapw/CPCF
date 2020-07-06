//////////////////////////////////////////////////////////////////////
// Cicada Project Shared APIs
//
// (C) Copyright Cicada DevTeam 2012.
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
// Absolutely no warranty!!
//////////////////////////////////////////////////////////////////////

#include <objc/objc.h>
#include "predefines.h"

//////////////////////////////////////////////////////
// Mac Stuffs
#if defined(PLATFORM_MAC)
#import <Cocoa/Cocoa.h>
#include "user_inputs.h"

void _objc_view_get_size(LPVOID pNSView, int* w, int *h)
{	*w = (int)(((__bridge NSView*)pNSView).bounds.size.width + 0.5f);
	*h = (int)(((__bridge NSView*)pNSView).bounds.size.height + 0.5f);
}

void _objc_view_accept_mousemove(LPVOID pNSView, bool set)
{	[[(__bridge NSView*)pNSView window] setAcceptsMouseMovedEvents: set];
}

bool _objc_pumpevents()
{
	bool hadevent = FALSE;
	for(;;)
	{
		NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
		if(event)
		{	hadevent = TRUE;
			[NSApp sendEvent:event];
#if !__has_feature(objc_arc)
			[event release];
#endif
		}
		else break;
	}
	return hadevent;
}

void _objc_preference_save_string(LPCSTR key, LPCSTR val)
{
	NSString* nskey = [[NSString alloc] initWithUTF8String:key];
	NSString* nsval = [[NSString alloc] initWithUTF8String:val];
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[defaults setObject:nsval forKey:nskey];
	[defaults synchronize];
#if !__has_feature(objc_arc)
	[nskey release];
	[nsval release];
#endif
}

UINT _objc_preference_load_string(LPCSTR key, LPSTR val_out, UINT val_size)
{
	NSString* nskey = [[NSString alloc] initWithUTF8String:key];
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSString *string = [defaults objectForKey:nskey];
	if(string == nil)return 0;
	
	const char* utf8_string = [string UTF8String];
	UINT len;
	if(utf8_string && ((len = (UINT)strlen(utf8_string)) < val_size))
    {	memcpy(val_out, utf8_string, len+1);
	}
	else
    {	len = 0;
	}
#if !__has_feature(objc_arc)
	[nskey release];
#endif
	return len;
}

UINT _objc_get_screens_dim(rt::Vec2i* p, UINT co)
{
    NSArray *screenArray = [NSScreen screens];
    UINT screenCount = (UINT)[screenArray count];
    UINT index = 0;
    for (; index < screenCount && index < co; index++)
    {
        NSScreen *screen = [screenArray objectAtIndex: index];
        NSRect screenRect = [screen visibleFrame];
        p[index].x = screenRect.size.width;
        p[index].y = screenRect.size.height;
    }
    
    return index;
}

#endif


//////////////////////////////////////////////////////
// iOS Stuffs
#if defined(PLATFORM_IOS)
#import <UIKit/UIDevice.h>
#import <UIKit/UIKit.h>
#import <AdSupport/ASIdentifierManager.h>
#include "user_inputs.h"

#define DispatchMainThreadAsync(block)\
if ([NSThread isMainThread]) {\
block();\
} else {\
dispatch_async(dispatch_get_main_queue(), block);\
}

int _objc_get_battery_state();
int _objc_get_battery_state()
{
	if([UIDevice currentDevice].batteryState == UIDeviceBatteryStateUnplugged)
	{	return (int)(100*[UIDevice currentDevice].batteryLevel);
	}
	else return 100;
}

int _objc_get_app_sandbox_path(char * path_out, int path_len);
int _objc_get_app_sandbox_path(char * path_out, int path_len)
{
	NSString* path = @"~/Documents";
	NSString* expanded = [path stringByExpandingTildeInPath];
	const char* utf8str = [expanded UTF8String];
	int len = (int)strlen(utf8str);
	if(path_len > len)
	{	memcpy(path_out, utf8str, len+1);
		return len;
	}
	else return 0;
}

void _objc_view_get_size(LPVOID pUIView, int* w, int *h);
void _objc_view_get_size(LPVOID pUIView, int* w, int *h)
{	*w = (int)(((__bridge UIView*)pUIView).bounds.size.width + 0.5f);
	*h = (int)(((__bridge UIView*)pUIView).bounds.size.height + 0.5f);
}

LPVOID __p_debug_textbox = NULL;

void _setup_debug_textbox(LPVOID pView);
void _setup_debug_textbox(LPVOID pView)
{
    __p_debug_textbox = pView;
}

void _set_debug_textbox(LPCSTR text);
void _set_debug_textbox(LPCSTR text)
{
    if(__p_debug_textbox)
    {
        [((__bridge UITextView*)__p_debug_textbox) setText:[NSString stringWithUTF8String:text] ];
    }
}

// add the AdSupport frameworks to your "Link with Binaries" build phase.
bool _objc_get_device_uid(char id[64]);
bool _objc_get_device_uid(char id[64])
{
	NSString *adId = NULL;
	const char* str = [adId UTF8String];
	int len;
	if(	(adId = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString]) &&
		(str = [adId UTF8String]) &&
	    (len = (int)strlen(str)) < 64
	)
	{	memcpy(id, str, len+1);
		return true;
	}
	return false;
}

void _objc_open_url(char *urlCString);
void _objc_open_url(char *urlCString) {
    NSString *urlString = [NSString stringWithUTF8String:urlCString];
    NSURL *URL = [NSURL URLWithString:urlString];
    DispatchMainThreadAsync(^{
        if (@available(iOS 10, *)) {
            [[UIApplication sharedApplication] openURL:URL options:@{} completionHandler:nil];
        } else {
            [[UIApplication sharedApplication] openURL:URL];
        }
    });
}

int _objc_get_bundle_path(char* pOut, int OutSize);

#endif


//////////////////////////////////////////////////
// shared stuffs

#if defined(PLATFORM_MAC) || defined(PLATFORM_IOS)

int _objc_get_bundle_path(char* pOut, int OutSize)
{
	NSString *tileDirectory = [[NSBundle mainBundle] executablePath];
	const char* utf8str = [tileDirectory UTF8String];
	int len = (int)strlen(utf8str);
	if(OutSize > len)
	{	memcpy(pOut, utf8str, len+1);
		return len;
	}
	else return 0;
}

#endif
