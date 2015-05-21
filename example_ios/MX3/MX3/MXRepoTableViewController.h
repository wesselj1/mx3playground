//
//  MXRepoTableViewController.h
//  MX3
//
//  Created by Joey Wessel on 5/20/15.
//
//

#import <UIKit/UIKit.h>
#import "gen/MX3Api.h"
#import "gen/MX3UserListVmObserver.h"

@interface MXRepoTableViewController : UITableViewController <MX3RepoListVmObserver>
- (instancetype) initWithApi:(id <MX3Api>) api;
- (void)onUpdate:(NSMutableArray *)changes newData:(id <MX3RepoListVm>)newData;

@end
