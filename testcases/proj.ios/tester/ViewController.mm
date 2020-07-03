
#import "ViewController.h"
#import "DeviceConsole.h"

// 把 C 字符串用 NSLog 输出，使得可以显示到屏幕
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
