
/*
 
 Copyright (c) 2015 James Mackenzie
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 */

#import "JMControlViewController.h"
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

@interface JMControlViewController ()

@end

@implementation JMControlViewController
@synthesize ble;
@synthesize protocol;

- (void)awakeFromNib
{
    [super awakeFromNib];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.

    self.navigationItem.hidesBackButton = YES;

    protocol = [[RBLProtocol alloc] init];
    protocol.delegate = self;
    protocol.ble = ble;
    
    NSLog(@"ControlView: viewDidLoad");
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
    if (length == 2) {
        NSInteger level = data[0];
        enum directions_t direction = data[1];
        [JMAlert soundLevelAlert:level doEnable:true];
        [JMAlert vibLevelAlert:level doEnable:true];
        NSLog(@"Recieved %ldm alert from hardware with direction %ld", (long)level, (long)direction);
    }
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    uint8_t pin = indexPath.row;
    
    if (pin_cap[pin] == PIN_CAPABILITY_NONE)
        return 0;
    
    return 60;
}

@end
