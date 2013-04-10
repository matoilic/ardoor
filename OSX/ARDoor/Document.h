//
//  Document.h
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface Document : NSDocument

- (IBAction)imageSelected:(id)sender;

@property (assign) IBOutlet NSImageView *imageView;

@end
