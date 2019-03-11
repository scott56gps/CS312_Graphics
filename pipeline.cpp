#include "definitions.h"
#include "coursefunctions.h"

/***********************************************
 * CLEAR_SCREEN
 * Sets the screen to the indicated color value.
 **********************************************/
void clearScreen(Buffer2D<PIXEL> & frame, PIXEL color = 0xff000000)
{
    int h = frame.height();
    int w = frame.width();
    for(int y = 0; y < h; y++)
    {
        for(int x = 0; x < w; x++)
        {
            frame[y][x] = color;
        }
    }
}

/************************************************************
 * UPDATE_SCREEN
 * Blits pixels from RAM to VRAM for rendering.
 ***********************************************************/
void SendFrame(SDL_Texture* GPU_OUTPUT, SDL_Renderer * ren, SDL_Surface* frameBuf) 
{
    SDL_UpdateTexture(GPU_OUTPUT, NULL, frameBuf->pixels, frameBuf->pitch);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, GPU_OUTPUT, NULL, NULL);
    SDL_RenderPresent(ren);
}

/*************************************************************
 * POLL_CONTROLS
 * Updates the state of the application based on:
 * keyboard, mouse, touch screen, gamepad inputs. 
 ************************************************************/
void processUserInputs(bool & running)
{
    SDL_Event e;
    int mouseX;
    int mouseY;
    while(SDL_PollEvent(&e)) 
    {
        if(e.type == SDL_QUIT) 
        {
            running = false;
        }
        if(e.key.keysym.sym == 'q' && e.type == SDL_KEYDOWN) 
        {
            running = false;
        }
        
        // Add Mouse Support
        if (e.type == SDL_MOUSEMOTION)
        {
            int cursor = SDL_ShowCursor(SDL_QUERY);

            if (cursor == SDL_DISABLE)
            {
                double mouseX = e.motion.xrel;
                double mouseY = e.motion.yrel;

                myCamera.yaw -= mouseX * 0.02;
                myCamera.pitch += mouseY * 0.02;
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            int cursor = SDL_ShowCursor(SDL_QUERY);

            if (cursor == SDL_DISABLE)
            {
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
            } else
            {
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        }

        // Translation
        if (e.key.keysym.sym == 'w' && e.type == SDL_KEYDOWN)
        {
            myCamera.z += (cos((myCamera.yaw / 180) * M_PI)) * 0.05;
            myCamera.x -= (sin((myCamera.yaw / 180) * M_PI)) * 0.05;
        }
        if (e.key.keysym.sym == 's' && e.type == SDL_KEYDOWN)
        {
            myCamera.z -= (cos((myCamera.yaw / 180) * M_PI)) * 0.05;
            myCamera.x += (sin((myCamera.yaw / 180) * M_PI)) * 0.05;
        }
        if (e.key.keysym.sym == 'a' && e.type == SDL_KEYDOWN)
        {
            myCamera.x -= (cos((myCamera.yaw / 180) * M_PI)) * 0.05;
            myCamera.z -= (sin((myCamera.yaw / 180) * M_PI)) * 0.05;
        }
        if (e.key.keysym.sym == 'd' && e.type == SDL_KEYDOWN)
        {
            myCamera.x += (cos((myCamera.yaw / 180) * M_PI)) * 0.05;
            myCamera.z += (sin((myCamera.yaw / 180) * M_PI)) * 0.05;
        }
    }
}

/****************************************
 * COMPUTE_DETERMINANT
 * Computes the determinant for a set of
 * 2 Verticies, given in Matrix Form
 * thus:
 * 
 * [a b]
 * [c d]
 * *************************************/
double computeDeterminant(double a, double b, double c, double d) {
    return (a * d) - (b * c);
}

/****************************************
 * INTERPOLATE
 * Interpolates (mixes) components from 3
 * colors.
 * 
 * area - Area of a primitive
 * firstDet - area of sub-triangle 1
 * ...
 * color1 - Color for attribute 1
 * ...
 * *************************************/
double interpolate(double area, double firstDet, double secondDet, double thirdDet, double attr1, double attr2, double attr3)
{
    // Divide the subtriangle area by the entire triangle area
    //  Then, multiply the result by whatever the attribute value is
    double component1 = (secondDet / area) * attr1;
    double component2 = (thirdDet / area) * attr2;
    double component3 = (firstDet / area) * attr3;

    // We add the components together to get a single interpolated attribute value
    return component1 + component2 + component3;
}

/****************************************
 * DRAW_POINT
 * Renders a point to the screen with the
 * appropriate coloring.
 ***************************************/
void DrawPoint(Buffer2D<PIXEL> & target, Vertex* v, Attributes* attrs, Attributes * const uniforms, FragmentShader* const frag)
{
    frag->FragShader(target[(int)v[0].y][(int)v[0].x], *attrs, *uniforms);
}

/****************************************
 * DRAW_LINE
 * Renders a line to the screen.
 ***************************************/
void DrawLine(Buffer2D<PIXEL> & target, Vertex* const triangle, Attributes* const attrs, Attributes* const uniforms, FragmentShader* const frag)
{
    // Your code goes here
}

/*************************************************************
 * DRAW_TRIANGLE
 * Renders a triangle to the target buffer. Essential 
 * building block for most of drawing.
 ************************************************************/
void DrawTriangle(Buffer2D<PIXEL> &target, Vertex* const triangle, Attributes* const attrs, Attributes* const uniforms, FragmentShader* const frag)
{
    // Make a BoundingBox for this triangle
    BoundingBox boundingBox(triangle[0], triangle[1], triangle[2]);

    // Make three sides (Vectors) of the triangle
    Vertex firstVect = {
        triangle[1].x - triangle[0].x,
        triangle[1].y - triangle[0].y,
        1,
        1
    };

    Vertex secondVect = {
        triangle[2].x - triangle[1].x,
        triangle[2].y - triangle[1].y,
        1,
        1
    };

    Vertex thirdVect = {
        triangle[0].x - triangle[2].x,
        triangle[0].y - triangle[2].y,
        1,
        1
    };

    double area = computeDeterminant(firstVect.x, -thirdVect.x, firstVect.y, -thirdVect.y);

    for (int y = boundingBox.minY; y <= boundingBox.maxY; y++) {
        for (int x = boundingBox.minX; x <= boundingBox.maxX; x++) {
            // These three determinants determine how much of each attribute we will
            //  give weight to in interpolation.
            double firstDet = computeDeterminant(firstVect.x, x - triangle[0].x, firstVect.y, y - triangle[0].y);
            double secondDet = computeDeterminant(secondVect.x, x - triangle[1].x, secondVect.y, y - triangle[1].y);
            double thirdDet = computeDeterminant(thirdVect.x, x - triangle[2].x, thirdVect.y, y - triangle[2].y);

            // Test if this point is in the triangle
            if ((firstDet >= 0) && (secondDet >= 0) && (thirdDet >= 0)) {
                // Attributes with which we will shade a fragment
                Attributes interpolatedAttributes;

                // Iterpolate the passed attributes
                interpolatedAttributes.values[0] = interpolate(area, firstDet, secondDet, thirdDet, attrs[0].values[0], attrs[1].values[0], attrs[2].values[0]);
                interpolatedAttributes.values[1] = interpolate(area, firstDet, secondDet, thirdDet, attrs[0].values[1], attrs[1].values[1], attrs[2].values[1]);
                interpolatedAttributes.values[2] = interpolate(area, firstDet, secondDet, thirdDet, attrs[0].values[2], attrs[1].values[2], attrs[2].values[2]);

                interpolatedAttributes.values[0] = interpolate(area, firstDet, secondDet, thirdDet, attrs[0].values[0], attrs[1].values[0], attrs[2].values[0]);
                interpolatedAttributes.values[1] = interpolate(area, firstDet, secondDet, thirdDet, attrs[0].values[1], attrs[1].values[1], attrs[2].values[1]);
                
                // Shade the fragment using the interpolated attributes and any uniforms
                frag->FragShader(target[y][x], interpolatedAttributes, *uniforms);
            }
        }
    }
}

/**************************************************************
 * VERTEX_SHADER_EXECUTE_VERTICES
 * Executes the vertex shader on inputs, yielding transformed
 * outputs. 
 *************************************************************/
void VertexShaderExecuteVertices(const VertexShader* vert, Vertex const inputVerts[], Attributes const inputAttrs[], const int& numIn, 
                                 Attributes* const uniforms, Vertex transformedVerts[], Attributes transformedAttrs[])
{
    // Defaults to pass-through behavior
    if(vert == NULL)
    {
        for(int i = 0; i < numIn; i++)
        {
            transformedVerts[i] = inputVerts[i];
            transformedAttrs[i] = inputAttrs[i];
        }
    } else {
        // For each inputVert, send it to the shader
        for (int i = 0; i < numIn; i++)
        {
            vert->VertShader(transformedVerts[i], transformedAttrs[i], inputVerts[i], inputAttrs[i], *uniforms);
        }
    }
}

void intersectAgainstYLimit(double &along, const double &yLimit, const double &segStartY, const double &segEndY)
{
    along = -1;

    double segDiffY = (segEndY - segStartY);
    if (segDiffY == 0)
    {
        return;
    }

    along = (yLimit - segStartY) / segDiffY;
}

void intersectAtPositiveLine(double &along, const double &segStartX, const double &segStartY, const double &segEndX, const double &segEndY)
{
    along = -1;

    double segDiffX = (segEndX - segStartX);
    double segDiffY = (segEndY - segStartY);

    if (segDiffX == segDiffY)
    {
        return;
    }

    along = (segStartY - segStartX) / (segDiffX - segDiffY);
}

void intersectAtNegativeLine(double &along, const double &segStartX, const double &segStartY, const double &segEndX, const double &segEndY)
{
    along = -1;

    double segDiffX = (segEndX - segStartX);
    double segDiffY = (segEndY - segStartY);

    if (segDiffX == segDiffY)
    {
        return;
    }

    along = (segStartY + segStartX) / (-segDiffX - segDiffY);
}

Vertex vertBetweenVerts(const Vertex &vertA, const Vertex &vertB, const double &along)
{
    Vertex returnVertex;
    returnVertex.x = (vertA.x) + ((vertB.x - vertA.x) * along);
    returnVertex.y = (vertA.y) + ((vertB.y - vertA.y) * along);
    returnVertex.z = (vertA.z) + ((vertB.z - vertA.z) * along);
    returnVertex.w = (vertA.w) + ((vertB.w - vertA.w) * along);

    return returnVertex;
}

void clipVertices(Vertex const transformedVerts[], Attributes const transformedAttrs[],
                const int &numIn, Vertex clippedVerts[], Attributes clippedAttrs[], int &numClipped)
{
    // TMP Clip buffers
    int num;
    int numOut;
    bool inBounds[MAX_VERTICES];
    Vertex tmpVertA[MAX_VERTICES];
    Vertex tmpVertB[MAX_VERTICES];
    Attributes tmpAttrA[MAX_VERTICES];
    Attributes tmpAttrB[MAX_VERTICES];

    Vertex const * srcVerts;
    Vertex * sinkVerts;
    Attributes const * srcAttrs;
    Attributes * sinkAttrs;

    // Setup Pointers for the first round of clipping
    srcVerts = transformedVerts;
    srcAttrs = transformedAttrs;
    sinkVerts = tmpVertA;
    sinkAttrs = tmpAttrA;

    num = numIn;
    numOut = 0;

    // CLIP against W (1st Pass)
    double wLimit = 0.001;  // Don't divide by zero.  This sets the limit
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = (srcVerts[i].w > wLimit);
    }
    for  (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAgainstYLimit(along, wLimit, srcVerts[current].w, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAgainstYLimit(along, wLimit, srcVerts[current].w, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Clipping against x
    srcVerts = tmpVertA;
    srcAttrs = tmpAttrA;
    sinkVerts = tmpVertB;
    sinkAttrs = tmpAttrB;
    num = numOut;
    numOut = 0;

    // Clipping against x = w (2nd Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = srcVerts[i].x < srcVerts[i].w;
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].x, srcVerts[current].w,
                                    srcVerts[next].x, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].x, srcVerts[current].w,
                                    srcVerts[next].x, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Setup Pointers
    srcVerts = tmpVertB;
    srcAttrs = tmpAttrB;
    sinkVerts = tmpVertA;
    sinkAttrs = tmpAttrA;
    num = numOut;
    numOut = 0;

    // Clipping against -x = w (3rd Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = (-srcVerts[i].x < srcVerts[i].w);
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].x, srcVerts[current].w,
                                    srcVerts[next].x, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].x, srcVerts[current].w,
                                    srcVerts[next].x, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Setup Pointers
    srcVerts = tmpVertB;
    srcAttrs = tmpAttrB;
    sinkVerts = tmpVertA;
    sinkAttrs = tmpAttrA;
    num = numOut;
    numOut = 0;

    // Clipping against y = w (4th Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = srcVerts[i].y < srcVerts[i].w;
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].y, srcVerts[current].w,
                                    srcVerts[next].y, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].y, srcVerts[current].w,
                                    srcVerts[next].y, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Setup Pointers
    srcVerts = tmpVertB;
    srcAttrs = tmpAttrB;
    sinkVerts = tmpVertA;
    sinkAttrs = tmpAttrA;
    num = numOut;
    numOut = 0;

    // Clipping against -y = w (5th Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = (-srcVerts[i].y < srcVerts[i].w);
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].y, srcVerts[current].w,
                                    srcVerts[next].y, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].y, srcVerts[current].w,
                                    srcVerts[next].y, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Setup Pointers
    srcVerts = tmpVertB;
    srcAttrs = tmpAttrB;
    sinkVerts = tmpVertA;
    sinkAttrs = tmpAttrA;
    num = numOut;
    numOut = 0;

    // Clipping against z = w (6th Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = srcVerts[i].z < srcVerts[i].w;
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].z, srcVerts[current].w,
                                    srcVerts[next].z, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtPositiveLine(along, srcVerts[current].z, srcVerts[current].w,
                                    srcVerts[next].z, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

    // Setup Pointers
    srcVerts = tmpVertB;
    srcAttrs = tmpAttrB;
    sinkVerts = clippedVerts;
    sinkAttrs = clippedAttrs;
    num = numOut;
    numOut = 0;

    // Clipping against -z = w (7th Pass)
    for (int i = 0; i < num; i++)
    {
        inBounds[i] = (-srcVerts[i].z < srcVerts[i].w);
    }
    for (int i = 0; i < num; i++)
    {
        int current = i;
        int next = (i+1) % num;
        bool currentVertexIn = inBounds[current];
        bool nextVertexIn = inBounds[next];

        if (currentVertexIn && nextVertexIn)
        {
            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else if (currentVertexIn && !nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].z, srcVerts[current].w,
                                    srcVerts[next].z, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);
        }
        else if (!currentVertexIn && nextVertexIn)
        {
            double along;
            intersectAtNegativeLine(along, srcVerts[current].z, srcVerts[current].w,
                                    srcVerts[next].z, srcVerts[next].w);
            sinkVerts[numOut] = vertBetweenVerts(srcVerts[current], srcVerts[next], along);
            sinkAttrs[numOut++] = Attributes(srcAttrs[current], srcAttrs[next], along);

            sinkVerts[numOut] = srcVerts[next];
            sinkAttrs[numOut++] = srcAttrs[next];
        }
        else
        {
            ; // Do Nothing
        }
    }

}

void normalizeVertices(Vertex clippedVerts[], Attributes clippedAttrs[], int const &numClipped)
{
    for (int i = 0; i < numClipped; i++)
    {
        clippedVerts[i].x /= clippedVerts[i].w;
        clippedVerts[i].y /= clippedVerts[i].w;
        clippedVerts[i].z /= clippedVerts[i].w;

        // Divide by the depth to make a perspective
        double zValue = clippedVerts[i].w;
        clippedVerts[i].w = 1.0 / clippedVerts[i].w;

        // Divide each attribute value by z
        for (int j = 0; j < clippedAttrs[i].numValues; j++)
        {
            clippedAttrs[i].values[j] /= zValue;
        }
    }
}

void viewportTransform(Buffer2D<PIXEL> &target, Vertex clippedVerts[], int &numClipped)
{
    // Move from -1 -> 1 space in X,Y to screen coordinates
    int w = target.width();
    int h = target.height();

    for (int i = 0; i < numClipped; i++)
    {
        clippedVerts[i].x = round((((clippedVerts[i].x + 1) / 2.0 * w)));
        clippedVerts[i].y = round((((clippedVerts[i].y + 1) / 2.0 * h)));
    }
}

/***************************************************************************
 * DRAW_PRIMITIVE
 * Processes the indicated PRIMITIVES type through pipeline stages of:
 *  1) Vertex Transformation
 *  2) Clipping
 *  3) Normalization
 *  4) ViewPort transform
 *  5) Rasterization & Fragment Shading
 **************************************************************************/
void DrawPrimitive(PRIMITIVES prim, 
                   Buffer2D<PIXEL>& target,
                   const Vertex inputVerts[],
                   const Attributes inputAttrs[],
                   Attributes* const uniforms,
                   FragmentShader* const frag,                   
                   VertexShader* const vert,
                   Buffer2D<double>* zBuf)
{
    // Setup count for vertices & attributes
    int numIn = 0;
    switch(prim)
    {
        case POINT:
            numIn = 1;
            break;
        case LINE:
            numIn = 2;
            break;
        case TRIANGLE:
            numIn = 3;
            break;
    }

    // Vertex shader 
    Vertex transformedVerts[MAX_VERTICES];
    Attributes transformedAttrs[MAX_VERTICES];
    VertexShaderExecuteVertices(vert, inputVerts, inputAttrs, numIn, uniforms, transformedVerts, transformedAttrs);

    // Clipping
    Vertex clippedVerts[MAX_VERTICES];
    Attributes clippedAttrs[MAX_VERTICES];
    int numClipped;
    clipVertices(transformedVerts, transformedAttrs, numIn, clippedVerts, clippedAttrs, numClipped);

    // Normalization
    // This is where we come from Homogenous Coordinates to screen cordinates
    normalizeVertices(clippedVerts, clippedAttrs, numClipped);

    // Viewport Transform
    // Transform our coordinates into a system that works in the viewport
    viewportTransform(target, clippedVerts, numClipped);

    // Vertex Interpolation & Fragment Drawing
    switch(prim)
    {
        case POINT:
            DrawPoint(target, clippedVerts, clippedAttrs, uniforms, frag);
            break;
        case LINE:
            DrawLine(target, clippedVerts, clippedAttrs, uniforms, frag);
            break;
        case TRIANGLE:
            Vertex tri[3];
            Attributes vAttr[3];
            for (int i = 2; i < numClipped; i++)
            {
                tri[0] = clippedVerts[0];
                tri[1] = clippedVerts[i-1];
                tri[2] = clippedVerts[i];

                vAttr[0] = clippedAttrs[0];
                vAttr[1] = clippedAttrs[i-1];
                vAttr[2] = clippedAttrs[i];
                
                DrawTriangle(target, tri, vAttr, uniforms, frag);
            }
    }
}

// Here, I will make a FragmentShader callback function
void GreenFragmentShader(PIXEL & fragment, const Attributes & vertexAttr, const Attributes & uniforms)
{
    // OK, so we will zero out the red and blue and just leave the rest
    //  which will be green
    
    fragment = vertexAttr.color & 0xff00ff00;
}

/*************************************************************
 * MAIN:
 * Main game loop, initialization, memory management
 ************************************************************/
int main()
{
    // -----------------------DATA TYPES----------------------
    SDL_Window* WIN;               // Our Window
    SDL_Renderer* REN;             // Interfaces CPU with GPU
    SDL_Texture* GPU_OUTPUT;       // GPU buffer image (GPU Memory)
    SDL_Surface* FRAME_BUF;        // CPU buffer image (Main Memory) 

    // ------------------------INITIALIZATION-------------------
    SDL_Init(SDL_INIT_EVERYTHING);
    WIN = SDL_CreateWindow(WINDOW_NAME, 200, 200, S_WIDTH, S_HEIGHT, 0);
    REN = SDL_CreateRenderer(WIN, -1, SDL_RENDERER_SOFTWARE);
    FRAME_BUF = SDL_CreateRGBSurface(0, S_WIDTH, S_HEIGHT, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    FRAME_BUF = SDL_ConvertSurface(SDL_GetWindowSurface(WIN), SDL_GetWindowSurface(WIN)->format, 0);
    GPU_OUTPUT = SDL_CreateTextureFromSurface(REN, FRAME_BUF);
    BufferImage frame(FRAME_BUF);
    // BufferImage bmpImage("../battletoads.bmp");

    // Draw loop 
    bool running = true;
    while(running) 
    {           
        // Handle user inputs
        processUserInputs(running);

        // Refresh Screen
        clearScreen(frame);

        TestPipeline(frame);

        // Push to the GPU
        SendFrame(GPU_OUTPUT, REN, FRAME_BUF);
    }

    // Cleanup
    SDL_FreeSurface(FRAME_BUF);
    SDL_DestroyTexture(GPU_OUTPUT);
    SDL_DestroyRenderer(REN);
    SDL_DestroyWindow(WIN);
    SDL_Quit();
    return 0;
}
