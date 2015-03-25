//
//  JMSoundsTableViewController.m
//  Third Eye
//
//  Created by Saleh Almuqbil on 2015-03-20.
//  Copyright (c) 2015 JamesMackenzie. All rights reserved.
//

#import "JMSoundsTableViewController.h"
#import "JMControlViewController.h"
#import <AudioToolbox/AudioServices.h>

@interface JMSoundsTableViewController ()

@end

@implementation JMSoundsTableViewController {
	NSArray *sounds;
	UITableViewCell *currentSelectedCell;
}

@synthesize delegate, selectedSound;

- (void)viewDidLoad {
    [super viewDidLoad];
	
	sounds = [NSArray arrayWithObjects:@"Tara",@"Tink - Key",@"Tink - Pin",@"Tock",@"Touch",@"Short-Low-High", nil];
	
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewWillDisappear:(BOOL)animated{
	if([self.navigationController.viewControllers indexOfObject:self]==NSNotFound){
		NSLog(@"BACK CLICKED");
		
		selectedSound = currentSelectedCell.textLabel.text;
		[[self delegate] setAlertSoundName:selectedSound];
	}
	[super viewWillDisappear:animated];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {

    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [sounds count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	static NSString *simpleID = @"SoundsCell";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleID];
	
	if (cell == nil) {
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:simpleID];
	}
	
	cell.textLabel.text = [sounds objectAtIndex:indexPath.row];
	if ([cell.textLabel.text isEqualToString:selectedSound]) {
		cell.accessoryType = UITableViewCellAccessoryCheckmark;
		currentSelectedCell = cell;
	}
	
	return cell;
}


-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
	long index = indexPath.row;
	
	currentSelectedCell.accessoryType = UITableViewCellAccessoryNone;
	UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
	cell.accessoryType = UITableViewCellAccessoryCheckmark;
	currentSelectedCell = cell;
	if (index == 0) {
		AudioServicesPlaySystemSound (1111); // Tara
	}else if (index == 1){
		AudioServicesPlaySystemSound (1103); // Tink - Key
	}else if (index == 2){
		AudioServicesPlaySystemSound (1057); // Tink - Pin
	}else if (index == 3){
		AudioServicesPlaySystemSound (1306); // Tock
	}else if (index == 4){
		AudioServicesPlaySystemSound (1202); // Touch
	}else if (index == 5){
		AudioServicesPlaySystemSound (1256); // Short-Low-High
	 }
	

}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


/*
// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/
@end
