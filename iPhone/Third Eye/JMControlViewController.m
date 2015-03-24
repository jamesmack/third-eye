
/*

 Copyright (c) 2015 James Mackenzie

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#import "JMControlViewController.h"
#import "JMSoundsTableViewController.h"
#import "CellPin.h"
#import "JMAlert.h"

uint8_t total_pin_count  = 0;
uint8_t pin_mode[128]    = {0};
uint8_t pin_cap[128]     = {0};
uint8_t pin_digital[128] = {0};
uint16_t pin_analog[128]  = {0};
uint8_t pin_pwm[128]     = {0};
uint8_t pin_servo[128]   = {0};

uint8_t init_done = 0;

NSInteger lastSelectedAlert=0;

@interface JMControlViewController ()

@end

@implementation JMControlViewController{
    NSArray *sounds;
    NSMutableArray *selectedAlerts;
    NSArray *alertSoundIDs;
}
@synthesize ble;
@synthesize protocol;
@synthesize alertToneString;

- (void)awakeFromNib
{
    [super awakeFromNib];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.

    selectedAlerts = [NSMutableArray arrayWithObjects:@"Tara",@"Tock",@"Short-Low-High", nil];

    /* Sound IDs */
    sounds = [NSArray arrayWithObjects:@"Tara",@"Tink - Key",@"Tink - Pin",@"Tock",@"Touch",@"Short-Low-High", nil];
    NSNumber *Tara = [NSNumber numberWithInt:1111];
    NSNumber *TinkK = [NSNumber numberWithInt:1103];
    NSNumber *TinkP = [NSNumber numberWithInt:1057];
    NSNumber *Tock = [NSNumber numberWithInt:1306];
    NSNumber *Touch = [NSNumber numberWithInt:1202];
    NSNumber *SLH = [NSNumber numberWithInt:1256];

    alertSoundIDs  = [NSArray arrayWithObjects: Tara, TinkK, TinkP, Tock, Touch, SLH, nil];


    self.alertOneLabel.text = selectedAlerts[0];
    self.alertTwoLabel.text = selectedAlerts[1];
    self.alertThreeLabel.text = selectedAlerts[2];

    self.navigationItem.hidesBackButton = YES;

    protocol = [[RBLProtocol alloc] init];
    protocol.delegate = self;
    protocol.ble = ble;

    NSLog(@"ControlView: viewDidLoad");
}

-(void) viewWillAppear:(BOOL)animated{
    if ([alertToneString length] > 0) {
        selectedAlerts[lastSelectedAlert] = alertToneString;
        self.alertOneLabel.text = selectedAlerts[0];
        self.alertTwoLabel.text = selectedAlerts[1];
        self.alertThreeLabel.text = selectedAlerts[2];
    }
}

NSTimer *syncTimer;

-(void) syncTimeout:(NSTimer *)timer
{
    NSLog(@"Timeout: no response");

    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Error"
                                                    message:@"Communication could not be established with Third Eye."
                                                   delegate:nil
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:nil];
    [alert show];

    // disconnect it
    [ble.CM cancelPeripheralConnection:ble.activePeripheral];
}

-(void)viewDidAppear:(BOOL)animated
{
    NSLog(@"ControlView: viewDidAppear");

    syncTimer = [NSTimer scheduledTimerWithTimeInterval:(float)3.0 target:self selector:@selector(syncTimeout:) userInfo:nil repeats:NO];

    [protocol queryProtocolVersion];
}

-(void)viewDidDisappear:(BOOL)animated
{
    NSLog(@"ControlView: viewDidDisappear");

    total_pin_count = 0;
    [tv reloadData];

    init_done = 0;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)btnStopClicked:(id)sender
{
    NSLog(@"Button Stop Clicked");

    [[ble CM] cancelPeripheralConnection:[ble activePeripheral]];
}

-(void) processData:(uint8_t *) data length:(uint8_t) length
{
#if defined(CV_DEBUG)
    NSLog(@"ControlView: processData");
    NSLog(@"Length: %d", length);
#endif

    [protocol parseData:data length:length];
}

-(void) protocolDidReceiveProtocolVersion:(uint8_t)major Minor:(uint8_t)minor Bugfix:(uint8_t)bugfix
{
    NSLog(@"protocolDidReceiveProtocolVersion: %d.%d.%d", major, minor, bugfix);

    // get response, so stop timer
    [syncTimer invalidate];
}

-(void) protocolDidReceiveTotalPinCount:(UInt8) count
{
    NSLog(@"protocolDidReceiveTotalPinCount: not implemented");
}

-(void) protocolDidReceivePinCapability:(uint8_t)pin Value:(uint8_t)value
{
    NSLog(@"protocolDidReceivePinCapability: not implemented");
}

-(void) protocolDidReceivePinData:(uint8_t)pin Mode:(uint8_t)mode Value:(uint8_t)value
{
    NSLog(@"protocolDidReceivePinData: not implemented");
}

-(void) protocolDidReceivePinMode:(uint8_t)pin Mode:(uint8_t)mode
{
    NSLog(@"protocolDidReceivePinMode: not implemented");
}

-(void) protocolDidReceiveCustomData:(uint8_t *)data length:(uint8_t)length
{
    if (length == 1) {
        NSInteger level = data[0];

        if (level == 1) {
            NSInteger alertSoundID = [[alertSoundIDs objectAtIndex:[sounds indexOfObject:_alertOneLabel.text]] integerValue];
            [JMAlert soundLevelAlert:level doEnable:true soundID:alertSoundID];
        }else if(level == 2){
            NSInteger alertSoundID = [[alertSoundIDs objectAtIndex:[sounds indexOfObject:_alertTwoLabel.text]] integerValue];
            [JMAlert soundLevelAlert:level doEnable:true soundID:alertSoundID];
        }else if(level == 3){
            NSInteger alertSoundID = [[alertSoundIDs objectAtIndex:[sounds indexOfObject:_alertThreeLabel.text]] integerValue];
            [JMAlert soundLevelAlert:level doEnable:true soundID:alertSoundID];
        }

        NSLog(@"Recieved alert %ld from hardware", (long)level);
    }
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {

    NSIndexPath *indexPath = [self.tableView indexPathForCell:sender];
    lastSelectedAlert = indexPath.row;

    JMSoundsTableViewController *soundPage = segue.destinationViewController;

    soundPage.title = [NSString stringWithFormat:@"Alert %d Sounds", lastSelectedAlert+1];

    soundPage.selectedSound = selectedAlerts[lastSelectedAlert];
    
    [soundPage setDelegate:self];
    
}

#pragma mark - Protocol Method

-(void)setAlertSoundName: (NSString*)alertName{
    alertToneString = alertName;
}

@end
