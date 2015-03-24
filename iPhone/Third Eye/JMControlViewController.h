
/*
 
 Copyright (c) 2015 James Mackenzie
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 */

#import <UIKit/UIKit.h>
#import "RBLProtocol.h"
#import "BLE.h"
#import "JMSoundsTableViewController.h"

enum directions_t {TOWARDS = 1, AWAY = 2, NOT_MOVING = 3, UNDETERMINED = 4};

@interface JMControlViewController : UITableViewController <ProtocolDelegate, passAlertSound>
{
    IBOutlet UITableView *tv;
}

@property (nonatomic, strong)NSString *alertToneString;

@property (strong, nonatomic) IBOutlet UILabel *alertOneLabel;

@property (strong, nonatomic) IBOutlet UILabel *alertTwoLabel;

@property (strong, nonatomic) IBOutlet UILabel *alertThreeLabel;

@property (strong, nonatomic) BLE *ble;
@property (strong, nonatomic) RBLProtocol *protocol;

-(void) processData:(uint8_t *) data length:(uint8_t) length;

@end
