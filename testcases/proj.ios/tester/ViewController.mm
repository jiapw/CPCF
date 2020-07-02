

#import "ViewController.h"
#include "../../../core/os/user_inputs.h"

os::UserInputEventPump _UIEventPump;

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    UIView* _pView = [self view];
    _UIEventPump.Init(_pView);
    
    os::SetupDebugTextBox(_pTextView);
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
}

-(IBAction) Reload
{
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Reload"
                                                message:@"Testing Message, lol."
                                                delegate:nil
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:nil];
    [alert show];
    [alert release];
    
    
    os::SetDebugTextBox("Hello World");
    os::SetDebugTextBox("Hello World 2");
    
    _UIEventPump.SendEvent_Command(1);
}

@end
