//
//  JMAlert.m
//  Third Eye
//
//  Created by James Mackenzie on 2015-02-24.
//  Copyright (c) 2015 JamesMackenzie. All rights reserved.
//

#import "JMAlert.h"
#import <AudioToolbox/AudioServices.h>

@implementation JMAlert

+ (void)soundLevelAlert: (NSInteger) level
              doEnable: (BOOL) enable
			  soundID: (NSInteger) soundNumber{
	AudioServicesPlayAlertSound ((unsigned int) soundNumber);
}

+ (void)vibLevelAlert: (NSInteger) level
            doEnable: (BOOL) enable {
    AudioServicesPlaySystemSound (4095);
}

@end
