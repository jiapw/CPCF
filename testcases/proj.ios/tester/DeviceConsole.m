//
//  DeviceConsole.m
//  ConsoleDemo
//
//  Created by yanggang on 2020/7/4.
//  Copyright © 2020 yanggang. All rights reserved.
//
#import "DeviceConsole.h"

@interface DeviceConsole ()

@property (nonatomic, weak) UIView *superView;
@property (nonatomic, assign) CGRect frame;
@property (nonatomic, strong) UITextView *textView;

@end

@implementation DeviceConsole

+ (void)showConsoleInView:(UIView *)superView frame:(CGRect)frame {
    [DeviceConsole sharedInstance].superView = superView;
    [DeviceConsole sharedInstance].frame = frame;
    [[DeviceConsole sharedInstance] showConsole];
}

+ (instancetype)sharedInstance {
    static DeviceConsole * sharedInstance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[DeviceConsole alloc] init];
    });
    return sharedInstance;
}

- (id)init {
    if (self = [super init]) {
        [self resetLogData];
    }
    return self;
}

- (void)resetLogData {
    [NSFileManager.defaultManager removeItemAtPath:[self logFilePath] error:nil];
    freopen([[self logFilePath] fileSystemRepresentation], "a", stderr);
}

- (NSString *)logFilePath {
    return [[DeviceConsole documentsDirectory] stringByAppendingPathComponent:@"ns.log"];
}

+ (NSString *)documentsDirectory {
	return [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)objectAtIndex:0];
}

- (void)showConsole {
	if (self.textView != nil) {
        return;
	}
    self.textView = [[UITextView alloc] initWithFrame:self.frame];
    self.textView.backgroundColor = UIColor.blackColor;
    self.textView.textColor = UIColor.whiteColor;
    self.textView.font = [UIFont systemFontOfSize:10];
    self.textView.editable = NO;
    [self.superView addSubview:self.textView];

    [self setUpToGetLogData];
    [self scrollToLast];
}

- (void)scrollToLast {
	NSRange txtOutputRange;
	txtOutputRange.location = self.textView.text.length;
	txtOutputRange.length = 0;
	self.textView.editable = YES;
	[self.textView scrollRangeToVisible:txtOutputRange];
    self.textView.selectedRange = txtOutputRange;
	self.textView.editable = NO;
}
- (void)setUpToGetLogData { 
	NSFileHandle *fileHandle = [NSFileHandle fileHandleForReadingAtPath:[self logFilePath]];
	[NSNotificationCenter.defaultCenter addObserver:self selector:@selector(getData:) name:@"NSFileHandleReadCompletionNotification" object:fileHandle];
	[fileHandle readInBackgroundAndNotify];
}

- (void)getData:(NSNotification *)notification {
	NSData *data = notification.userInfo[NSFileHandleNotificationDataItem];
	if (data.length == 0) {
        [self performSelector:@selector(refreshLog:) withObject:notification afterDelay:1.0];
        return;
	}
	NSString *string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    self.textView.editable = YES;
    self.textView.text = [self.textView.text stringByAppendingString:string];
    self.textView.editable = NO;
    double delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void) {
        [self scrollToLast];
    });
    [notification.object readInBackgroundAndNotify];
}

- (void)refreshLog:(NSNotification *)notification {
	[notification.object readInBackgroundAndNotify];
}

@end
