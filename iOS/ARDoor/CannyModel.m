//
//  CannyModel.m
//  ARDoor
//
//  Created by Mato Ilic on 06.03.13.
//  Copyright (c) 2013 Mato Ilic. All rights reserved.
//

#import "CannyModel.h"

@interface CannyModel () {
    unsigned int screenWidth;
    unsigned int screenHeight;
    unsigned int poolWidth;
    unsigned int poolHeight;
    
    unsigned int meshFactor;
    
    float texCoordFactorS;
    float texCoordOffsetS;
    float texCoordFactorT;
    float texCoordOffsetT;
    
    // simulation buffers
    float *source;
    float *dest;
    
    // data passed to GL
    GLfloat *vertices;
    GLfloat *texCoords;
    GLushort *indicies;
}

@end

@implementation CannyModel

- (void)dealloc
{
    [self freeBuffers];
}

- (void)freeBuffers
{
    free(source);
    free(dest);
    
    free(vertices);
    free(texCoords);
    free(indicies);
}


- (unsigned int)getIndexCount
{
    return [self getIndexSize]/sizeof(*indicies);
}

- (unsigned int)getIndexSize
{
    return (poolHeight-1)*(poolWidth*2+2)*sizeof(GLushort);
}

- (GLushort *)getIndices
{
    return indicies;
}

- (GLfloat *)getVertices
{
    return vertices;
}

- (GLfloat *)getTexCoords
{
    return texCoords;
}

- (unsigned int)getVertexSize
{
    return poolWidth*poolHeight*2*sizeof(GLfloat);
}

- (void)initMap
{
    // +2 for padding the border
    memset(source, 0, (poolWidth+2)*(poolHeight+2)*sizeof(float));
    memset(dest, 0, (poolWidth+2)*(poolHeight+2)*sizeof(float));
}

- (void)initMesh
{
    for (int i = 0; i < poolHeight; i++)
    {
        for (int j = 0; j < poolWidth; j++)
        {
            vertices[(i*poolWidth+j)*2+0] = -1.f + j*(2.f/(poolWidth-1));
            vertices[(i*poolWidth+j)*2+1] = 1.f - i*(2.f/(poolHeight-1));
            
            texCoords[(i*poolWidth+j)*2+0] = (float)i/(poolHeight-1) * texCoordFactorS + texCoordOffsetS;
            texCoords[(i*poolWidth+j)*2+1] = (1.f - (float)j/(poolWidth-1)) * texCoordFactorT + texCoordFactorT;
        }
    }
    
    unsigned int index = 0;
    for (int i = 0; i < poolHeight - 1; i++)
    {
        for (int j = 0; j < poolWidth; j++)
        {
            if (i % 2 == 0)
            {
                // emit extra index to create degenerate triangle
                if (j == 0)
                {
                    indicies[index] = i*poolWidth+j;
                    index++;
                }
                
                indicies[index] = i*poolWidth+j;
                index++;
                indicies[index] = (i+1)*poolWidth+j;
                index++;
                
                // emit extra index to create degenerate triangle
                if (j == (poolWidth-1))
                {
                    indicies[index] = (i+1)*poolWidth+j;
                    index++;
                }
            }
            else
            {
                // emit extra index to create degenerate triangle
                if (j == 0)
                {
                    indicies[index] = (i+1)*poolWidth+j;
                    index++;
                }
                
                indicies[index] = (i+1)*poolWidth+j;
                index++;
                indicies[index] = i*poolWidth+j;
                index++;
                
                // emit extra index to create degenerate triangle
                if (j == (poolWidth-1))
                {
                    indicies[index] = i*poolWidth+j;
                    index++;
                }
            }
        }
    }
}

- (id)initWithScreenWidth:(unsigned int)width
             screenHeight:(unsigned int)height
               meshFactor:(unsigned int)factor
             textureWidth:(unsigned int)texWidth
            textureHeight:(unsigned int)texHeight
{
    self = [super init];
    
    if (self)
    {
        screenWidth = width;
        screenHeight = height;
        meshFactor = factor;
        poolWidth = width/meshFactor;
        poolHeight = height/meshFactor;
        
        if ((float)screenHeight/screenWidth < (float)texWidth/texHeight)
        {
            texCoordFactorS = (float)(texHeight*screenHeight)/(screenWidth*texWidth);
            texCoordOffsetS = (1.f - texCoordFactorS)/2.f;
            
            texCoordFactorT = 1.f;
            texCoordOffsetT = 0.f;
        }
        else
        {
            texCoordFactorS = 1.f;
            texCoordOffsetS = 0.f;
            
            texCoordFactorT = (float)(screenWidth*texWidth)/(texHeight*screenHeight);
            texCoordOffsetT = (1.f - texCoordFactorT)/2.f;
        }
        
        // +2 for padding the border
        source = (float *)malloc((poolWidth+2)*(poolHeight+2)*sizeof(float));
        dest = (float *)malloc((poolWidth+2)*(poolHeight+2)*sizeof(float));
        
        vertices = (GLfloat *)malloc(poolWidth*poolHeight*2*sizeof(GLfloat));
        texCoords = (GLfloat *)malloc(poolWidth*poolHeight*2*sizeof(GLfloat));
        indicies = (GLushort *)malloc((poolHeight-1)*(poolWidth*2+2)*sizeof(GLushort));
        
        if (!source || !dest ||
            !vertices || !texCoords || !indicies)
        {
            [self freeBuffers];
            return nil;
        }
        
        [self initMap];
        
        [self initMesh];
    }
    
    return self;
}

- (void)runSimulation
{
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // first pass for simulation buffers...
    dispatch_apply(poolHeight, queue, ^(size_t y) {
        for (int x = 0; x < poolWidth; x++)
        {
            // * - denotes current pixel
            //
            //       a
            //     c * d
            //       b
            
            // +1 to both x/y values because the border is padded
            float a = source[(y)*(poolWidth+2) + x+1];
            float b = source[(y+2)*(poolWidth+2) + x+1];
            float c = source[(y+1)*(poolWidth+2) + x];
            float d = source[(y+1)*(poolWidth+2) + x+2];
            
            float result = (a + b + c + d)/2.f - dest[(y+1)*(poolWidth+2) + x+1];
            
            result -= result/32.f;
            
            dest[(y+1)*(poolWidth+2) + x+1] = result;
        }
    });
    
    // second pass for modifying texture coord
    dispatch_apply(poolHeight, queue, ^(size_t y) {
        for (int x = 0; x < poolWidth; x++)
        {
            // * - denotes current pixel
            //
            //       a
            //     c * d
            //       b
            
            // +1 to both x/y values because the border is padded
            float a = dest[(y)*(poolWidth+2) + x+1];
            float b = dest[(y+2)*(poolWidth+2) + x+1];
            float c = dest[(y+1)*(poolWidth+2) + x];
            float d = dest[(y+1)*(poolWidth+2) + x+2];
            
            float s_offset = ((b - a) / 2048.f);
            float t_offset = ((c - d) / 2048.f);
            
            // clamp
            s_offset = (s_offset < -0.5f) ? -0.5f : s_offset;
            t_offset = (t_offset < -0.5f) ? -0.5f : t_offset;
            s_offset = (s_offset > 0.5f) ? 0.5f : s_offset;
            t_offset = (t_offset > 0.5f) ? 0.5f : t_offset;
            
            float s_tc = (float)y/(poolHeight-1) * texCoordFactorS + texCoordOffsetS;
            float t_tc = (1.f - (float)x/(poolWidth-1)) * texCoordFactorT + texCoordOffsetT;
            
            texCoords[(y*poolWidth+x)*2+0] = s_tc + s_offset;
            texCoords[(y*poolWidth+x)*2+1] = t_tc + t_offset;
        }
    });
    
    float *pTmp = dest;
    dest = source;
    source = pTmp;
}

@end
