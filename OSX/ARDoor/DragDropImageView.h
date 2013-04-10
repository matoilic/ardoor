//
//  DragDropImageView.h
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CameraCalibration.h"

@interface DragDropImageView : NSImageView <NSDraggingSource, NSDraggingDestination, NSPasteboardItemDataProvider>

@end
