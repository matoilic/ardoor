//
//  DisortionImageView.h
//  ARDoor
//
//  Created by Mato Ilic on 17.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "DragDropImageView.h"
#import "CameraCalibration.h"
#import "DragDropImageView.h"

@interface DisortionImageView : DragDropImageView
{
    ARDoor::CameraCalibration *_calibrator;
}

@property (nonatomic, assign) ARDoor::CameraCalibration *calibrator;

@end
