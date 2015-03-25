//
//  JMAlert.m
//  Third Eye
//
//  Created by James Mackenzie on 2015-02-24.
//  Copyright (c) 2015 JamesMackenzie. All rights reserved.
//

#import "JMAlert.h"
#import <AudioToolbox/AudioServices.h>
@import MediaPlayer;

@implementation JMAlert {
    CGFloat _prevAudioLevel;
    BOOL _isSoundLowered;
    MPMusicPlayerController *_musicPlayer;
}

- (id)init {
    self = [super init];

    if (self) {
        _musicPlayer = [MPMusicPlayerController applicationMusicPlayer];
        _isSoundLowered = NO;
        _prevAudioLevel = _musicPlayer.volume;
    }

    return self;
}

- (void)soundLevelAlert: (NSInteger) level
              doEnable: (BOOL) enable
			  soundID: (NSInteger) soundNumber{
	AudioServicesPlayAlertSound ((unsigned int) soundNumber);
}

- (void)vibLevelAlert: (NSInteger) level
            doEnable: (BOOL) enable {
    AudioServicesPlaySystemSound (4095);
}

- (void)lowerAudioLevel: (NSInteger) byPercent{
    _prevAudioLevel = _musicPlayer.volume;
    _musicPlayer.volume = _prevAudioLevel*byPercent/100;
    _isSoundLowered = YES;
}

- (void)restoreAudioLevel {
    _musicPlayer.volume = _prevAudioLevel;
    _isSoundLowered = NO;
}

- (BOOL)isSoundLowered {
    return _isSoundLowered;
}

@end
