//
//  MXRepoTableViewController.m
//  MX3
//
//  Created by Joey Wessel on 5/20/15.
//
//

#import "MXRepoTableViewController.h"
#import "gen/MX3RepoListVmCell.h"
#import "gen/MX3ListChange.h"
#import "MXRepoTableViewCell.h"
#import "UIImageView+AFNetworking.h"

NSString *const CellIdentifier2 = @"MX3RepoCell";

@interface MXRepoTableViewController ()

@property (nonatomic) id <MX3Api> api;
@property (nonatomic) id <MX3RepoListVmHandle> handle;
@property (nonatomic) id <MX3RepoListVm> viewModel;

@end

@implementation MXRepoTableViewController

- (instancetype) initWithApi:(id<MX3Api>)api {
    self = [super initWithStyle:UITableViewStylePlain];
    if (self) {
        self.api = api;
        self.handle = [api observerRepoList];
        self.viewModel = nil;
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = [self.api getUsername];
    [self.tableView registerNib:[UINib nibWithNibName:@"MXUserTableViewCell" bundle:[NSBundle mainBundle]] forCellReuseIdentifier:CellIdentifier2];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    // todo(piet) what is the proper 'weak-ref' style here
    [self.handle start:self];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [self.handle stop];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source
- (void)onUpdate:(NSMutableArray *)changes newData:(id <MX3RepoListVm>)newData {
    if (changes) {
        [self.tableView beginUpdates];
        for (MX3ListChange *change in changes) {
            if (change.fromIndex >= 0 && change.toIndex >= 0) {
                NSUInteger indexes[] = {0, change.toIndex};
                NSIndexPath *path = [NSIndexPath indexPathWithIndexes:indexes length:2];
                [self.tableView reloadRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
            } else if (change.fromIndex >= 0) {
                NSUInteger indexes[] = {0, change.fromIndex};
                NSIndexPath *path = [NSIndexPath indexPathWithIndexes:indexes length:2];
                [self.tableView deleteRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
            } else {
                NSUInteger indexes[] = {0, change.toIndex};
                NSIndexPath *path = [NSIndexPath indexPathWithIndexes:indexes length:2];
                [self.tableView insertRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
            }
        }
        self.viewModel = newData;
        [self.tableView endUpdates];
    } else {
        self.viewModel = newData;
        [self.tableView reloadData];
    }
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.viewModel != nil ? [self.viewModel count] : 0;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    MXRepoTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier2 forIndexPath:indexPath];
    MX3RepoListVmCell * cellData = [self.viewModel get:(int32_t)indexPath.row];
    cell.lblName.text = cellData.name;
    cell.lblLanguage.text = cellData.language;
    cell.lblStargazers.text = [NSString stringWithFormat:@"%d", cellData.stargazersCount];
    cell.lblWatchers.text = [NSString stringWithFormat:@"%d", cellData.watchersCount];
    return cell;
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
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
