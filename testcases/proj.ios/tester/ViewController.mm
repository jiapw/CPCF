#import "ViewController.h"
#import "DeviceConsole.h"

void logCString(char *chars) {
    NSString *string = [NSString stringWithUTF8String:chars];
    NSLog(@"%@", string);
}

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [DeviceConsole showConsoleInView:self.view];
    
    char *test = "test";
    logCString(test);
}

@end
