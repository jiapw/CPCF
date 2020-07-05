#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "../../../core/os/file_dir.h"

int main(int argc, char *argv[])
{
    os::CommandLine cmd(argc, argv);
    
	@autoreleasepool {
	    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
	}
}
