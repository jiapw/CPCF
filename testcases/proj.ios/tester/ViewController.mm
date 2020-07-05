
#import "ViewController.h"
#import "DeviceConsole.h"
#include "../../../core/os/multi_thread.h"
#include "../../tests/test.h"
#import "TXTUIMacros.h"

void ConsoleWriter(LPCSTR log, int type, LPVOID cookie)
{
    NSString *string = [NSString stringWithUTF8String:log];
    NSLog(@"%@", string);
}

extern "C" void TestMain();

@interface ViewController () {
    os::Thread  _Worker;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CGRect frame = self.view.bounds;
    frame = CGRectMake(0, mStatusBarHeight, frame.size.width, frame.size.height - mStatusBarHeight - mTabbarAdditionalHeight);
    [DeviceConsole showConsoleInView:self.view frame:frame];
    
    os::_details::SetConsoleLogWriteFunction(ConsoleWriter, nullptr);
    
    _Worker.Create([](){
        TestMain();
        _LOG("\n\nAll tests are done.");
    });
}

@end
