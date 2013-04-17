//
//  DisortionImageView.m
//  ARDoor
//
//  Created by Mato Ilic on 17.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "DisortionImageView.h"
#import "NSImage+OpenCV.h"

@implementation DisortionImageView

@synthesize calibrator;

- (void)setCalibrator:(ARDoor::CameraCalibration *)c
{
    _calibrator = c;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{    
    if ([sender draggingSource] != self)
    {
        NSURL* fileURL;
        
        //set the image using the best representation we can get from the pasteboard
        if([NSImage canInitWithPasteboard: [sender draggingPasteboard]])
        {
            NSImage *newImage = [[NSImage alloc] initWithPasteboard: [sender draggingPasteboard]];
            cv::Mat mat = [newImage CVGrayscaleMat];
            cv::Size size = cv::Size(mat.cols, mat.rows);
            _calibrator->calibrate(size);
            _calibrator->remap(mat);
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
