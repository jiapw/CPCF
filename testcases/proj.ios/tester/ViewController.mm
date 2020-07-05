
#import "ViewController.h"
#import "DeviceConsole.h"
#import "TXTUIMacros.h"

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
    
    CGRect frame = self.view.bounds;
    frame = CGRectMake(0, mStatusBarHeight, frame.size.width, frame.size.height - mStatusBarHeight - mTabbarAdditionalHeight);
    [DeviceConsole showConsoleInView:self.view frame:frame];
    
    char *test = "test";
    logCString(test);
}

@end
