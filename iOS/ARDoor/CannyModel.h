//
//  CannyModel.h
//  ARDoor
//
//  Created by Mato Ilic on 06.03.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CannyModel : NSObject

- (GLfloat *)getVertices;
- (GLfloat *)getTexCoords;
- (GLushort *)getIndices;
- (unsigned int)getVertexSize;
- (unsigned int)getIndexSize;
- (unsigned int)getIndexCount;

- (id)initWithScreenWidth:(unsigned int)width
             screenHeight:(unsigned int)height
               meshFactor:(unsigned int)factor
             textureWidth:(unsigned int)texWidth
            textureHeight:(unsigned int)texHeight;

- (void)runSimulation;

@end
