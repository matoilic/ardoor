//
//  Configuration.h
//  ARDoor
//
//  Created by Mato Ilic on 19.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Configuration : NSObject
{
    NSNumber *_boardColumns;
    NSNumber *_boardRows;
}

@property (nonatomic, readonly) NSNumber *boardColumns;
@property (nonatomic, readonly) NSNumber *boardRows;

+(int)boardColumns;
+(int)boardRows;

@end
