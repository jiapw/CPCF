//
//  ConsoleService.m
//  tester
//
//  Created by yanggang on 2020/7/5.
//

#import "ConsoleService.h"

@implementation ConsoleService

+ (void)logString:(NSString *)string type:(ConsoleLogType)type {
    NSLog(@"%@%@%02ld%@%@", DeviceConsolePrefix, DeviceConsoleLogType, (long)type, DeviceConsoleLogString, string);
}

@end
