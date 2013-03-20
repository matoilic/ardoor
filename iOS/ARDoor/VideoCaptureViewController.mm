
#import "UIImage+OpenCV.h"
#import "VideoCaptureViewController.h"

// Number of frames to average for FPS calculation
const int kFrameTimeBufferSize = 25;

const int kCannyAperture = 7;
const int kCannyLow = 20 * kCannyAperture * kCannyAperture;
const int kCannyHigh = 200 * kCannyAperture * kCannyAperture;

@interface VideoCaptureViewController ()
    - (BOOL)createCaptureSessionForCamera:(NSInteger)camera qualityPreset:(NSString *)qualityPreset grayscale:(BOOL)grayscale;

    - (void)destroyCaptureSession;

    - (void)processFrame:(cv::Mat&)mat videoRect:(CGRect)rect videoOrientation:(AVCaptureVideoOrientation)orientation;

    - (void)updateDebugInfo;

    - (void)showEdges:(const cv::Mat &)edgeData
            forVideoRect:(CGRect)videoRect
            videoOrientation:(AVCaptureVideoOrientation)videoOrientation;

    @property (nonatomic, assign) float fps;
@end

@implementation VideoCaptureViewController

@synthesize fps = _fps;
@synthesize camera = _camera;
@synthesize captureGrayscale = _captureGrayscale;
@synthesize qualityPreset = _qualityPreset;
@synthesize captureSession = _captureSession;
@synthesize captureDevice = _captureDevice;
@synthesize videoOutput = _videoOutput;
@synthesize videoPreviewLayer = _videoPreviewLayer;

@dynamic showDebugInfo;
@dynamic torchOn;
@dynamic showEdges;

// Create an affine transform for converting CGPoints and CGRects from the video frame coordinate space to the
// preview layer coordinate space. Usage:
//
// CGPoint viewPoint = CGPointApplyAffineTransform(videoPoint, transform);
// CGRect viewRect = CGRectApplyAffineTransform(videoRect, transform);
//
// Use CGAffineTransformInvert to create an inverse transform for converting from the view cooridinate space to
// the video frame coordinate space.
//
// videoFrame: a rect describing the dimensions of the video frame
// video orientation: the video orientation
//
// Returns an affine transform
//
- (CGAffineTransform)affineTransformForVideoFrame:(CGRect)videoFrame orientation:(AVCaptureVideoOrientation)videoOrientation
{
    CGSize viewSize = self.view.bounds.size;
    NSString * const videoGravity = _videoPreviewLayer.videoGravity;
    CGFloat widthScale = 1.0f;
    CGFloat heightScale = 1.0f;
    
    // Move origin to center so rotation and scale are applied correctly
    CGAffineTransform t = CGAffineTransformMakeTranslation(-videoFrame.size.width / 2.0f, -videoFrame.size.height / 2.0f);
    
    switch (videoOrientation) {
        case AVCaptureVideoOrientationPortrait:
            widthScale = viewSize.width / videoFrame.size.width;
            heightScale = viewSize.height / videoFrame.size.height;
            break;
            
        case AVCaptureVideoOrientationPortraitUpsideDown:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(M_PI));
            widthScale = viewSize.width / videoFrame.size.width;
            heightScale = viewSize.height / videoFrame.size.height;
            break;
            
        case AVCaptureVideoOrientationLandscapeRight:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(M_PI_2));
            widthScale = viewSize.width / videoFrame.size.height;
            heightScale = viewSize.height / videoFrame.size.width;
            break;
            
        case AVCaptureVideoOrientationLandscapeLeft:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(-M_PI_2));
            widthScale = viewSize.width / videoFrame.size.height;
            heightScale = viewSize.height / videoFrame.size.width;
            break;
    }
    
    // Adjust scaling to match video gravity mode of video preview
    if (videoGravity == AVLayerVideoGravityResizeAspect) {
        heightScale = MIN(heightScale, widthScale);
        widthScale = heightScale;
    }
    else if (videoGravity == AVLayerVideoGravityResizeAspectFill) {
        heightScale = MAX(heightScale, widthScale);
        widthScale = heightScale;
    }
    
    // Apply the scaling
    t = CGAffineTransformConcat(t, CGAffineTransformMakeScale(widthScale, heightScale));
    
    // Move origin back from center
    t = CGAffineTransformConcat(t, CGAffineTransformMakeTranslation(viewSize.width / 2.0f, viewSize.height / 2.0f));
    
    return t;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput
        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:(AVCaptureConnection *)connection
{
    NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
    
    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    OSType format = CVPixelBufferGetPixelFormatType(pixelBuffer);
    CGRect videoRect = CGRectMake(0.0f, 0.0f, CVPixelBufferGetWidth(pixelBuffer), CVPixelBufferGetHeight(pixelBuffer));
    AVCaptureVideoOrientation videoOrientation = [[[_videoOutput connections] objectAtIndex:0] videoOrientation];
    
    if (format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
        // For grayscale mode, the luminance channel of the YUV data is used
        CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        void *baseaddress = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
        
        cv::Mat mat(videoRect.size.height, videoRect.size.width, CV_8UC1, baseaddress, 0);
        
        [self processFrame:mat videoRect:videoRect videoOrientation:videoOrientation];
        
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    }
    else if (format == kCVPixelFormatType_32BGRA) {
        // For color mode a 4-channel cv::Mat is created from the BGRA data
        CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        void *baseaddress = CVPixelBufferGetBaseAddress(pixelBuffer);
        
        cv::Mat mat(videoRect.size.height, videoRect.size.width, CV_8UC4, baseaddress, 0);
        
        [self processFrame:mat videoRect:videoRect videoOrientation:videoOrientation];
        
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    }
    else {
        NSLog(@"Unsupported video format");
    }
    
    // Update FPS calculation
    CMTime presentationTime = CMSampleBufferGetOutputPresentationTimeStamp(sampleBuffer);
    
    if (_lastFrameTimestamp == 0) {
        _lastFrameTimestamp = presentationTime.value;
        _framesToAverage = 1;
    }
    else {
        float frameTime = (float)(presentationTime.value - _lastFrameTimestamp) / presentationTime.timescale;
        _lastFrameTimestamp = presentationTime.value;
        
        _frameTimes[_frameTimesIndex++] = frameTime;
        
        if (_frameTimesIndex >= kFrameTimeBufferSize) {
            _frameTimesIndex = 0;
        }
        
        float totalFrameTime = 0.0f;
        for (int i = 0; i < _framesToAverage; i++) {
            totalFrameTime += _frameTimes[i];
        }
        
        float averageFrameTime = totalFrameTime / _framesToAverage;
        float fps = 1.0f / averageFrameTime;
        
        if (fabsf(fps - _captureQueueFps) > 0.1f) {
            _captureQueueFps = fps;
            dispatch_async(dispatch_get_main_queue(), ^{
                [self setFps:fps];
            });
        }
        
        _framesToAverage++;
        if (_framesToAverage > kFrameTimeBufferSize) {
            _framesToAverage = kFrameTimeBufferSize;
        }
    }
    
    [localpool drain];
}

// camera: -1 for default, 0 for back camera, 1 for front camera
// qualityPreset: [AVCaptureSession sessionPreset] value
// grayscale: YES to capture grayscale frames, NO to capture RGBA frames
//
- (BOOL)createCaptureSessionForCamera:(NSInteger)camera qualityPreset:(NSString *)qualityPreset grayscale:(BOOL)grayscale
{
    _lastFrameTimestamp = 0;
    _frameTimesIndex = 0;
    _captureQueueFps = 0.0f;
    _fps = 0.0f;
	
    // Set up AV capture
    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    
    if ([devices count] == 0) {
        NSLog(@"No video capture devices found");
        return NO;
    }
    
    if (camera == -1) {
        _camera = -1;
        _captureDevice = [[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] retain];
    }
    else if (camera >= 0 && camera < [devices count]) {
        _camera = camera;
        _captureDevice = [[devices objectAtIndex:camera] retain];
    }
    else {
        _camera = -1;
        _captureDevice = [[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] retain];
        NSLog(@"Camera number out of range. Using default camera");
    }
    
    // Create the capture session
    _captureSession = [[AVCaptureSession alloc] init];
    _captureSession.sessionPreset = (qualityPreset)? qualityPreset : AVCaptureSessionPresetMedium;
    
    // Create device input
    NSError *error = nil;
    AVCaptureDeviceInput *input = [[AVCaptureDeviceInput alloc] initWithDevice:_captureDevice error:&error];
    
    // Create and configure device output
    _videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    
    dispatch_queue_t queue = dispatch_queue_create("cameraQueue", NULL);
    [_videoOutput setSampleBufferDelegate:self queue:queue];
    dispatch_release(queue);
    
    _videoOutput.alwaysDiscardsLateVideoFrames = YES;
    _videoOutput.minFrameDuration = CMTimeMake(1, 30);
    
    
    // For grayscale mode, the luminance channel from the YUV fromat is used
    // For color mode, BGRA format is used
    OSType format = kCVPixelFormatType_32BGRA;
    
    // Check YUV format is available before selecting it (iPhone 3 does not support it)
    if (grayscale && [_videoOutput.availableVideoCVPixelFormatTypes containsObject:
                      [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange]]) {
        format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
    }
    
    _videoOutput.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:format]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    
    // Connect up inputs and outputs
    if ([_captureSession canAddInput:input]) {
        [_captureSession addInput:input];
    }
    
    if ([_captureSession canAddOutput:_videoOutput]) {
        [_captureSession addOutput:_videoOutput];
    }
    
    [input release];
    
    // Create the preview layer
    _videoPreviewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:_captureSession];
    [_videoPreviewLayer setFrame:self.view.bounds];
    _videoPreviewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    [self.view.layer insertSublayer:_videoPreviewLayer atIndex:0];
    
    return YES;
}

- (void)dealloc
{
    [self destroyCaptureSession];
    [_fpsLabel release];
    _fpsLabel = nil;
    
    if (_frameTimes) {
        free(_frameTimes);
    }
    
    [super dealloc];
}

- (void)destroyCaptureSession
{
    [_captureSession stopRunning];
    
    [_videoPreviewLayer removeFromSuperlayer];
    [_videoPreviewLayer release];
    [_videoOutput release];
    [_captureDevice release];
    [_captureSession release];
    
    _videoPreviewLayer = nil;
    _videoOutput = nil;
    _captureDevice = nil;
    _captureSession = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
    if (self) {
        _camera = -1;
        _qualityPreset = AVCaptureSessionPresetMedium;
        _captureGrayscale = YES;
        
        // Create frame time circular buffer for calculating averaged fps
        _frameTimes = (float*)malloc(sizeof(float) * kFrameTimeBufferSize);
        
        self.captureGrayscale = YES;
        self.qualityPreset = AVCaptureSessionPresetMedium;
    }
    
    return self;
}

// Switch camera 'on-the-fly'
//
// camera: 0 for back camera, 1 for front camera
//
- (void)setCamera:(int)camera
{
    if (camera != _camera)
    {
        _camera = camera;
        
        if (_captureSession) {
            NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
            
            [_captureSession beginConfiguration];
            
            [_captureSession removeInput:[[_captureSession inputs] lastObject]];
            
            [_captureDevice release];
            
            if (_camera >= 0 && _camera < [devices count]) {
                _captureDevice = [devices objectAtIndex:camera];
            }
            else {
                _captureDevice = [[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] retain];
            }
            
            // Create device input
            NSError *error = nil;
            AVCaptureDeviceInput *input = [[AVCaptureDeviceInput alloc] initWithDevice:_captureDevice error:&error];
            [_captureSession addInput:input];
            
            [_captureSession commitConfiguration];
        }
    }
}

- (void)setFps:(float)fps
{
    [self willChangeValueForKey:@"fps"];
    _fps = fps;
    [self didChangeValueForKey:@"fps"];
    
    [self updateDebugInfo];
}

- (void)setShowDebugInfo:(BOOL)showDebugInfo
{
    if (!showDebugInfo && _fpsLabel) {
        [_fpsLabel removeFromSuperview];
        [_fpsLabel release];
        _fpsLabel = nil;
    }
    
    if (showDebugInfo && !_fpsLabel) {
        // Create label to show FPS
        CGRect frame = self.view.bounds;
        frame.size.height = 40.0f;
        _fpsLabel = [[UILabel alloc] initWithFrame:frame];
        _fpsLabel.textColor = [UIColor whiteColor];
        _fpsLabel.backgroundColor = [UIColor colorWithWhite:0.0f alpha:0.5f];
        [self.view addSubview:_fpsLabel];
        
        [self updateDebugInfo];
    }
}

- (void)setTorchOn:(BOOL)torch
{
    NSError *error = nil;
    if ([_captureDevice hasTorch]) {
        BOOL locked = [_captureDevice lockForConfiguration:&error];
        if (locked) {
            _captureDevice.torchMode = (torch)? AVCaptureTorchModeOn : AVCaptureTorchModeOff;
            [_captureDevice unlockForConfiguration];
        }
    }
}

- (void)setShowEdges:(BOOL)showEdges
{
    if(showEdges)
    {
        [self.view.layer insertSublayer:_featureLayer above:_videoPreviewLayer];
    }
    else
    {
        [_featureLayer removeFromSuperlayer];
    }
    
    _showEdges = showEdges;
}

- (BOOL)showEdges
{
    return _showEdges;
}

- (BOOL)showDebugInfo
{
    return (_fpsLabel != nil);
}

- (BOOL)torchOn
{
    return (_captureDevice.torchMode == AVCaptureTorchModeOn);
}

- (void)updateDebugInfo {
    if (_fpsLabel) {
        _fpsLabel.text = [NSString stringWithFormat:@"FPS: %0.1f", _fps];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self createCaptureSessionForCamera:_camera qualityPreset:_qualityPreset grayscale:_captureGrayscale];
    [_captureSession startRunning];
    
    _edgeImage = [UIImage alloc];
    _featureLayer = [[CALayer alloc] init];
    
    self.showEdges = YES;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    [self destroyCaptureSession];
    [_fpsLabel release];
    _fpsLabel = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

// MARK: IBActions

// Toggles display of FPS
- (IBAction)toggleFps:(id)sender
{
    self.showDebugInfo = !self.showDebugInfo;
}

// Turn torch on and off
- (IBAction)toggleTorch:(id)sender
{
    self.torchOn = !self.torchOn;
}
  
// Switch between front and back camera
- (IBAction)toggleCamera:(id)sender
{
    if (self.camera == 1)
    {
        self.camera = 0;
    }
    else
    {
        self.camera = 1;
    }
}

- (IBAction)toggleEdges:(id)sender
{
    self.showEdges = !self.showEdges;
}

// MARK: VideoCaptureViewController overrides

- (void)processFrame:(cv::Mat &)mat videoRect:(CGRect)rect videoOrientation:(AVCaptureVideoOrientation)videOrientation
{
    if(!self.showEdges)
    {
        return;
    }
    
    // Shrink video frame to 320X240
    cv::resize(mat, mat, cv::Size(), 0.5f, 0.5f, CV_INTER_LINEAR);
    rect.size.width /= 2.0f;
    rect.size.height /= 2.0f;
    
    // Rotate video frame by 90deg to portrait by combining a transpose and a flip
    // Note that AVCaptureVideoDataOutput connection does NOT support hardware-accelerated
    // rotation and mirroring via videoOrientation and setVideoMirrored properties so we
    // need to do the rotation in software here.
    cv::transpose(mat, mat);
    CGFloat temp = rect.size.width;
    rect.size.width = rect.size.height;
    rect.size.height = temp;
    
    if (videOrientation == AVCaptureVideoOrientationLandscapeRight)
    {
        // flip around y axis for back camera
        cv::flip(mat, mat, 1);
    }
    else {
        // Front camera output needs to be mirrored to match preview layer so no flip is required here
    }
    
    videOrientation = AVCaptureVideoOrientationPortrait;
    
    cv::Sobel(mat, mat, mat.depth(), 1, 1, 3);
    cv::Canny(mat, mat, kCannyLow, kCannyHigh, kCannyAperture);
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [self showEdges:mat
           forVideoRect:rect
       videoOrientation:videOrientation];
    });
}

- (void) showEdges:(const cv::Mat &)edgeData
      forVideoRect:(CGRect)videoRect
  videoOrientation:(AVCaptureVideoOrientation)videoOrientation
{
	[CATransaction begin];
	[CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    
    [_edgeImage initWithCVMat:edgeData];
    
    CGAffineTransform t = [self affineTransformForVideoFrame:videoRect orientation:videoOrientation];
    
    CGRect edgeRect = CGRectMake(0, 0, videoRect.size.width, videoRect.size.height);
    
    edgeRect = CGRectApplyAffineTransform(edgeRect, t);
    
    _featureLayer.contents = (id)_edgeImage.CGImage;
    _featureLayer.frame = edgeRect;
    [_featureLayer setHidden:NO];
    
    [CATransaction commit];
}

@end
