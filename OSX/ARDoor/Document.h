//
//  Document.h
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CameraCalibration.h"
#import "CalibrationImageView.h"
#import "DisortionImageView.h"

@interface Document : NSDocument
{
    ARDoor::CameraCalibration *_calibrator;
}

- (IBAction)chooseFolder:(id)sender;

@property (assign) IBOutlet CalibrationImageView *calibrationImageView;
@property (assign) IBOutlet DisortionImageView *disortionImageView;

@end
