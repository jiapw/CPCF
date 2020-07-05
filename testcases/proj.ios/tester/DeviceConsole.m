//
//  DeviceConsole.m
//  ConsoleDemo
//
//  Created by yanggang on 2020/7/4.
//  Copyright © 2020 yanggang. All rights reserved.
//
#import "DeviceConsole.h"

NSString * const DeviceConsolePrefix = @"[DeviceConsole]";
NSString * const DeviceConsoleLogType = @"[LogType]";
NSString * const DeviceConsoleLogString = @"[LogString]";

static const NSUInteger kLogTypeStringLenth = 2;

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
    self.textView.attributedText = [self attributedStringWithNewString:string];
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

#pragma mark - log text ui

- (NSAttributedString *)attributedStringWithNewString:(NSString *)newString {
    if (newString.length == 0) {
        return self.textView.attributedText;
    }
    NSArray<NSString *> *strings = [newString componentsSeparatedByString:@"\n"];
    
    NSMutableAttributedString *totalString = [[NSMutableAttributedString alloc] init];
    [totalString appendAttributedString:self.textView.attributedText];
    
    [strings enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        NSRange prefixRange = [obj rangeOfString:DeviceConsolePrefix];
        if (prefixRange.location != NSNotFound) {
            obj = [obj substringFromIndex:prefixRange.location + prefixRange.length];
        }
        
        UIColor *color = UIColor.whiteColor;
        NSRange logTypeRange = [obj rangeOfString:DeviceConsoleLogType];
        if (logTypeRange.location != NSNotFound) {
            NSUInteger typeStartIndex = logTypeRange.location + logTypeRange.length;
            NSAssert((typeStartIndex + kLogTypeStringLenth) < obj.length, @"typeStartIndex + kLogTypeStringLenth out of range");
            NSString *typeString = [obj substringWithRange:NSMakeRange(typeStartIndex, kLogTypeStringLenth)];
            color = [self colorWithLogType:typeString.integerValue];
            obj = [obj substringFromIndex:logTypeRange.location + logTypeRange.length];
        }
        
        NSRange logStringRange = [obj rangeOfString:DeviceConsoleLogString];
        if (logStringRange.location != NSNotFound) {
            obj = [obj substringFromIndex:logStringRange.location + logStringRange.length];
            obj = [NSString stringWithFormat:@"%@\n", obj];
        }
        
        NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:obj];
        [string addAttribute:NSForegroundColorAttributeName value:color range:NSMakeRange(0, obj.length)];
        [totalString appendAttributedString:string];
    }];
    
    return totalString;
}

- (UIColor *)colorWithLogType:(ConsoleLogType)logType {
    switch (logType) {
        case ConsoleLogTypeVerbose: return UIColor.lightGrayColor;
        case ConsoleLogTypeHighlight: return UIColor.greenColor;
        case ConsoleLogTypeWarning: return UIColor.yellowColor;
        case ConsoleLogTypeError: return UIColor.redColor;
        default: return UIColor.whiteColor;
    }
}

@end
