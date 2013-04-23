//
//  DragDropImageView.m
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "CalibrationImageView.h"
#import "NSImage+OpenCV.h"
#import "Configuration.h"

@implementation CalibrationImageView

@synthesize calibrator;

- (ARDoor::CameraCalibration *)calibrator
{
    return _calibrator;
}

- (void)setCalibrator:(ARDoor::CameraCalibration *)c
{
    _calibrator = c;
}

#pragma MARK - Drag & Drop

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;
    cv::Size size = cv::Size([Configuration boardColumns], [Configuration boardRows]);
    
    if ([sender draggingSource] != self)
    {
        NSURL* fileURL;
        
        //set the image using the best representation we can get from the pasteboard
        if([NSImage canInitWithPasteboard: [sender draggingPasteboard]])
        {
            NSImage *newImage = [[NSImage alloc] initWithPasteboard: [sender draggingPasteboard]];
            cv::Mat mat = [newImage CVGrayscaleMat];
            if(_calibrator->findChessboardPoints(mat, size, imageCorners, objectCorners))
            {
                _calibrator->addPoints(imageCorners, objectCorners);
                NSLog(@"success");
            }
            cv::drawChessboardCorners(mat, size, imageCorners, true);
            //cv::Canny(mat, mat, 30, 150, 3);
            [self setImage:[newImage initWithCVMat:mat]];
        }
        
        //if the drag comes from a file, set the window title to the filename
        fileURL=[NSURL URLFromPasteboard: [sender draggingPasteboard]];
        [[self window] setTitle: fileURL!=NULL ? [fileURL absoluteString] : @"(no name)"];
        [self setNeedsDisplay: YES];
    }
    
    return NO; //do not copy the original image
}

@end
