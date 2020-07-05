//
//  TXTUIMacros.h
//  tester
//
//  Created by yanggang on 2020/7/5.
//

#ifndef TXTUIMacros_h
#define TXTUIMacros_h

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#endif

#define mScreenScale ([UIScreen mainScreen].scale)
#define mScreenWidth ([UIScreen mainScreen].bounds.size.width)
#define mScreenHeight ([UIScreen mainScreen].bounds.size.height)

#define mIs5_8InchScreen (mScreenHeight == 812 && mScreenWidth == 375)
#define mIs6_1InchScreen (mScreenHeight == 896 && mScreenWidth == 414)
#define mIs6_5InchScreen (mScreenHeight == 896 && mScreenWidth == 414)
#define mIsIphoneX (mIs5_8InchScreen || mIs6_1InchScreen || mIs6_5InchScreen)

#define mStatusBarHeight (mIsIphoneX ? 44:20)
#define mTabBarHeight (mIsIphoneX ? mIphoneXTabbarHeight:49)
#define mTabbarAdditionalHeight (mIsIphoneX ? 34 : 0)

#endif /* TXTUIMacros_h */
