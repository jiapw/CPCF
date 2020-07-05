//
//  ConsoleService.h
//  tester
//
//  Created by yanggang on 2020/7/5.
//

#import <Foundation/Foundation.h>
#import "DeviceConsole.h"

NS_ASSUME_NONNULL_BEGIN

@interface ConsoleService : NSObject

+ (void)logString:(NSString *)string type:(ConsoleLogType)type;

@end

NS_ASSUME_NONNULL_END
