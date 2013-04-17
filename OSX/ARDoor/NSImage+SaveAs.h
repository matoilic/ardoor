//
//  NSImage+SaveAs.h
//  ARDoor
//
//  Created by Mato Ilic on 17.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSImage (NSImage_SaveAs)
- (void) saveAsJpegWithName:(NSURL*) fileName;
@end
