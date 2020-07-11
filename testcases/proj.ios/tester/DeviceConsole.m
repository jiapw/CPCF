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

#pragma mark - log text ui

+ (void)logString:(NSString *)string type:(ConsoleLogType)type {
    [[self sharedInstance] logString:string type:type];
}

- (void)logString:(NSString *)string type:(ConsoleLogType)type {
    self.textView.editable = YES;
    self.textView.attributedText = [self attributedStringWithNewString:string type:type];
    self.textView.editable = NO;
    double delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void) {
        [self scrollToLast];
    });
}

- (NSAttributedString *)attributedStringWithNewString:(NSString *)newString type:(ConsoleLogType)type {
    if (newString.length == 0) {
        return self.textView.attributedText;
    }
    newString = [NSString stringWithFormat:@"%@\n", newString];
    NSMutableAttributedString *totalString = [[NSMutableAttributedString alloc] init];
    [totalString appendAttributedString:self.textView.attributedText];
    NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:newString];
    UIColor *color = [self colorWithLogType:type];
    [string addAttribute:NSForegroundColorAttributeName value:color range:NSMakeRange(0, newString.length)];
    [string addAttribute:NSFontAttributeName value:[self font] range:NSMakeRange(0, newString.length)];
    [totalString appendAttributedString:string];
    return totalString;
}

- (UIFont *)font {
    return [UIFont fontWithName:@"Menlo-Regular" size:11];
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
