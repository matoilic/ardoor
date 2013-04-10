//
//  DragDropImageView.m
//  ARDoor
//
//  Created by Mato Ilic on 05.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "DragDropImageView.h"
#import "NSImage+OpenCV.h"

@implementation DragDropImageView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        [self registerForDraggedTypes:[NSImage imagePasteboardTypes]];
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
}

- (NSRect)windowWillUseStandardFrame:(NSWindow *)window defaultFrame:(NSRect)newFrame
{
    /*------------------------------------------------------
     delegate operation to set the standard window frame
     --------------------------------------------------------*/
    //get window frame size
    NSRect ContentRect=self.window.frame;
    
    //set it to the image frame size
    ContentRect.size=[[self image] size];
    
    return [NSWindow frameRectForContentRect:ContentRect styleMask: [window styleMask]];
}

#pragma MARK - Drag & Drop

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{    
    // Check if the pasteboard contains image data and source/user wants it copied
    if ( [NSImage canInitWithPasteboard:[sender draggingPasteboard]] &&
         [sender draggingSourceOperationMask] &
         NSDragOperationCopy )
    {
        
        [self setNeedsDisplay: YES];
        
        //accept data as a copy operation
        return NSDragOperationCopy;
    }
    
    return NSDragOperationNone;
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{    
    [self setNeedsDisplay: YES];
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
    return [NSImage canInitWithPasteboard: [sender draggingPasteboard]];
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    NSLog(@"Image dropped");
    
    if ([sender draggingSource] != self)
    {
        NSURL* fileURL;
        
        //set the image using the best representation we can get from the pasteboard
        if([NSImage canInitWithPasteboard: [sender draggingPasteboard]])
        {
            NSImage *newImage = [[NSImage alloc] initWithPasteboard: [sender draggingPasteboard]];
            cv::Mat mat = [newImage CVGrayscaleMat];
            cv::Canny(mat, mat, 30, 150, 3);
            [self setImage:[newImage initWithCVMat:mat]];
        }
        
        //if the drag comes from a file, set the window title to the filename
        fileURL=[NSURL URLFromPasteboard: [sender draggingPasteboard]];
        [[self window] setTitle: fileURL!=NULL ? [fileURL absoluteString] : @"(no name)"];
        [self setNeedsDisplay: YES];
    }
    
    return NO;
}

- (void)pasteboard:(NSPasteboard *)sender item:(NSPasteboardItem *)item provideDataForType:(NSString *)type
{
    if ( [type compare: NSPasteboardTypeTIFF] == NSOrderedSame )
    {
        [sender setData:[[self image] TIFFRepresentation] forType:NSPasteboardTypeTIFF];
    }
    else if ( [type compare: NSPasteboardTypePDF] == NSOrderedSame )
    {
        [sender setData:[self dataWithPDFInsideRect:[self bounds]] forType:NSPasteboardTypePDF];
    }
}

@end
