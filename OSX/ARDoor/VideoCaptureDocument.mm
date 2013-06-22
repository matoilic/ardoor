//
//  VideoCapture.m
//  ARDoor
//
//  Created by Mato Ilic on 17.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <QuartzCore/QuartzCore.h>
#import "VideoCaptureDocument.h"
#import "NSImage+OpenCV.h"
#import "Configuration.h"

@implementation VideoCaptureDocument

- (id)init
{
    self = [super init];
    if (self) {
        _session = [[AVCaptureSession alloc] init];
        _outputImage = [[NSImage alloc] init];
        _calibrator = new ARDoor::CameraCalibration();
    }
    return self;
}

- (void)close
{
    delete(_calibrator);
    [_session stopRunning];
    [super close];
}

- (NSString *)windowNibName
{
    return @"VideoCaptureDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)aController
{
    [super windowControllerDidLoadNib:aController];
    
    AVCaptureDevice *videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    
    _videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    dispatch_queue_t queue = dispatch_queue_create("cameraQueue", NULL);
    [_videoOutput setSampleBufferDelegate:self queue:queue];
    _videoOutput.alwaysDiscardsLateVideoFrames = YES;
    _videoOutput.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    
    [_session beginConfiguration];
	[_session setSessionPreset:AVCaptureSessionPreset1280x720];
    
    NSError *error = nil;
    
    AVCaptureDeviceInput *videoDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    if (videoDeviceInput == nil) {
        dispatch_async(dispatch_get_main_queue(), ^(void) {
            [self presentError:error];
        });
    } else {        
        [_session addInput:videoDeviceInput];
    }
	
    [_session addOutput:_videoOutput];
	[_session commitConfiguration];
    
    [_session startRunning];

}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to write your document to data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning nil.
    // You can also choose to override -fileWrapperOfType:error:, -writeToURL:ofType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
    if (outError) {
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    }
    return nil;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    // Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error when returning NO.
    // You can also choose to override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
    // If you override either of these, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
    if (outError) {
        *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:unimpErr userInfo:NULL];
    }
    return YES;
}

+ (BOOL)autosavesInPlace
{
    return NO;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection
{    
    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CGRect videoRect = CGRectMake(0.0f, 0.0f, CVPixelBufferGetWidth(pixelBuffer), CVPixelBufferGetHeight(pixelBuffer));
    
    // For color mode a 4-channel cv::Mat is created from the BGRA data
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    void *baseaddress = CVPixelBufferGetBaseAddress(pixelBuffer);
    cv::Mat mat(videoRect.size.height, videoRect.size.width, CV_8UC4, baseaddress, 0);
    
    [self processFrame:mat videoRect:videoRect];
    
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
}

- (void)processFrame:(cv::Mat &)mat videoRect:(CGRect)rect
{
    if(![_session isRunning] || self.videoView == nil)
    {
        return;
    }
    
    std::vector<cv::Point2f> imageCorners;
    std::vector<cv::Point3f> objectCorners;
    cv::Size size = cv::Size([Configuration boardColumns], [Configuration boardRows]);
    
    [CATransaction begin];
	[CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    
    float scaleFactor = self.videoView.bounds.size.width / 1280.0f;
    cv::resize(mat, mat, cv::Size(), scaleFactor, scaleFactor, CV_INTER_LINEAR);
    _calibrator->findAndDrawChessboardPoints(mat, size, imageCorners, objectCorners);
    rect.size.width *= scaleFactor;
    rect.size.height *= scaleFactor;

    cv::cvtColor(mat, mat, CV_BGR2RGB);
    _outputImage = [_outputImage initWithCVMat:mat];
    self.videoView.image = _outputImage;
    self.videoView.needsDisplay = YES;
    
    [CATransaction commit];
}

@end
