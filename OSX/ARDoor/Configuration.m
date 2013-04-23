//
//  Configuration.m
//  ARDoor
//
//  Created by Mato Ilic on 19.04.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "Configuration.h"

@interface Configuration ()

+(Configuration*)instance;

@end

@implementation Configuration

@synthesize boardColumns = _boardColumns;
@synthesize boardRows = _boardRows;

-(id)init
{
    self = [super init];
    
    if(self)
    {
        NSBundle* mainBundle;
        mainBundle = [NSBundle mainBundle];
        _boardRows = [mainBundle objectForInfoDictionaryKey:@"BoardRows"];
        _boardColumns = [mainBundle objectForInfoDictionaryKey:@"BoardColumns"];
    }
    
    return self;
}

+(Configuration*)instance
{
    static Configuration *instance;
    if(instance == NULL)
    {
        instance = [[Configuration alloc] init];
    }
    
    return instance;
}

+(int)boardColumns
{
    return [self instance].boardColumns.intValue;
}

+(int)boardRows
{
    return [self instance].boardRows.intValue;
}

@end
