//
//  VideoCapture.h
//  ARDoor
//
//  Created by Mato Ilic on 17.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import "CameraCalibration.h"

@interface VideoCaptureDocument : NSDocument<AVCaptureVideoDataOutputSampleBufferDelegate>
{
@private
    AVCaptureSession *_session;
    AVCaptureVideoDataOutput *_videoOutput;
    NSImage *_outputImage;
    ARDoor::CameraCalibration *_calibrator;
}

@property (assign) IBOutlet NSImageView *videoView;

@end
