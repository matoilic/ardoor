//
//  DragDropImageView.h
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CameraCalibration.h"
#import "DragDropImageView.h"

@interface CalibrationImageView : DragDropImageView
{
    ARDoor::CameraCalibration *_calibrator;
    NSNumber *boardColumns;
    NSNumber *boardRows;
}

@property (nonatomic, assign) ARDoor::CameraCalibration *calibrator;

@end
