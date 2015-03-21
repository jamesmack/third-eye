//
//  JMAlert.h
//  Third Eye
//
//  Created by James Mackenzie on 2015-02-24.
//  Copyright (c) 2015 JamesMackenzie. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface JMAlert : NSObject

+ (void)soundLevelAlert: (NSInteger) level
              doEnable: (BOOL) enable
				soundID: (NSInteger) soundNumber;

+ (void)vibLevelAlert: (NSInteger) level
            doEnable: (BOOL) enable;

@end
