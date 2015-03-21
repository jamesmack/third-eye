//
//  JMSoundsTableViewController.h
//  Third Eye
//
//  Created by Saleh Almuqbil on 2015-03-20.
//  Copyright (c) 2015 JamesMackenzie. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol passAlertSound <NSObject>

-(void)setAlertSoundName: (NSString*)alertName;

@end

@interface JMSoundsTableViewController : UITableViewController

@property (nonatomic, strong) NSString *selectedSound;

@property (retain)id <passAlertSound> delegate;

@end
