//
//  DeviceConsole.h
//  ConsoleDemo
//
//  Created by yanggang on 2020/7/4.
//  Copyright © 2020 yanggang. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef NS_ENUM(NSInteger, ConsoleLogType) {
    ConsoleLogTypeVerbose = 0,
    ConsoleLogTypeUpdating,
    ConsoleLogTypeInformational,
    ConsoleLogTypeHighlight,
    ConsoleLogTypeWarning,
    ConsoleLogTypeError,
};

extern NSString * const DeviceConsolePrefix;
extern NSString * const DeviceConsoleLogType;
extern NSString * const DeviceConsoleLogString;

@interface DeviceConsole : NSObject

+ (void)showConsoleInView:(UIView *)superView frame:(CGRect)frame;

@end

