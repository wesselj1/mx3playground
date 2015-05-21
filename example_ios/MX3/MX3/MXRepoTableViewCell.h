//
//  MXRepoTableViewCell.h
//  MX3
//
//  Created by Joey Wessel on 5/20/15.
//
//

#import <UIKit/UIKit.h>

@interface MXRepoTableViewCell : UITableViewCell

@property (weak, nonatomic) IBOutlet UILabel *lblName;
@property (weak, nonatomic) IBOutlet UILabel *lblLanguage;
@property (weak, nonatomic) IBOutlet UILabel *lblStargazers;
@property (weak, nonatomic) IBOutlet UILabel *lblWatchers;

@end
