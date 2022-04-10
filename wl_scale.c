// WL_SCALE.C

#include "wl_def.h"


/*
=============================================================================

                              SPRITE DRAWING ROUTINES

=============================================================================
*/


/*
===================
=
= ScaleLine
=
= Reconstruct a sprite and draw it
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line)
= 	top of virtual line with segment in proper place
=	start of segment pixel*2
=	<repeat>
=
===================
*/

void ScaleLine (int16_t x, int16_t toppix, fixed fracstep, byte *linesrc, byte *linecmds, byte *curshades)
{
    byte    *src,*dest;
    byte    col;
    int16_t start,end,top;
    int16_t startpix,endpix;
    fixed   frac;

    for (end = READWORD(linecmds) >> 1; end; end = READWORD(linecmds) >> 1)
    {
        top = READWORD(linecmds + 2);
        start = READWORD(linecmds + 4) >> 1;

        frac = start * fracstep;

        endpix = (frac >> FRACBITS) + toppix;

        for (src = &linesrc[top + start]; start != end; start++, src++)
        {
            startpix = endpix;

            if (startpix >= viewheight)
                break;                          // off the bottom of the view area

            frac += fracstep;
            endpix = (frac >> FRACBITS) + toppix;

            if (endpix < 0)
                continue;                       // not into the view area

            if (startpix < 0)
                startpix = 0;                   // clip upper boundary

            if (endpix > viewheight)
                endpix = viewheight;            // clip lower boundary
                col = *src;

            dest = vbuf + ylookup[startpix] + x;

            while (startpix < endpix)
            {
                *dest = col;
                dest += bufferPitch;
                startpix++;
            }
        }

        linecmds += 6;                          // next segment list
    }
}


/*
===================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
===================
*/

void ScaleShape (int xcenter, int shapenum, int height, uint32_t flags)
{
    int         i;
    compshape_t *shape;
    byte        *linesrc,*linecmds;
    byte        *curshades;
    int16_t     scale,toppix;
    int16_t     x1,x2,actx;
    fixed       frac,fracstep;

    scale = height >> 3;        // low three bits are fractional

    if (!scale)
        return;                 // too close or far away

    linesrc = PM_GetSpritePage(shapenum);
    shape = (compshape_t *)linesrc;

    fracstep = FixedDiv(scale,TEXTURESIZE/2);
    frac = shape->leftpix * fracstep;

    actx = xcenter - scale;
    toppix = centery - scale;

    x2 = (frac >> FRACBITS) + actx;

    for (i = shape->leftpix; i <= shape->rightpix; i++)
    {
        //
        // calculate edges of the shape
        //
        x1 = x2;

        if (x1 >= viewwidth)
            break;                // off the right side of the view area

        frac += fracstep;
        x2 = (frac >> FRACBITS) + actx;

        if (x2 < 0)
            continue;             // not into the view area

        if (x1 < 0)
            x1 = 0;               // clip left boundary

        if (x2 > viewwidth)
            x2 = viewwidth;       // clip right boundary

        while (x1 < x2)
        {
            if (wallheight[x1] < height)
            {
                linecmds = &linesrc[shape->dataofs[i - shape->leftpix]];

                ScaleLine (x1,toppix,fracstep,linesrc,linecmds,curshades);
            }

            x1++;
        }
    }
}


/*
===================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
===================
*/

void SimpleScaleShape (int xcenter, int shapenum, int height)
{
    int         i;
    compshape_t *shape;
    byte        *linesrc,*linecmds;
    int16_t     scale,toppix;
    int16_t     x1,x2,actx;
    fixed       frac,fracstep;

    scale = height >> 1;

    linesrc = PM_GetSpritePage(shapenum);
    shape = (compshape_t *)linesrc;

    fracstep = FixedDiv(scale,TEXTURESIZE/2);
    frac = shape->leftpix * fracstep;

    actx = xcenter - scale;
    toppix = centery - scale;

    x2 = (frac >> FRACBITS) + actx;

    for (i = shape->leftpix; i <= shape->rightpix; i++)
    {
        //
        // calculate edges of the shape
        //
        x1 = x2;

        frac += fracstep;
        x2 = (frac >> FRACBITS) + actx;

        while (x1 < x2)
        {
            linecmds = &linesrc[shape->dataofs[i - shape->leftpix]];

            ScaleLine (x1,toppix,fracstep,linesrc,linecmds,NULL);

            x1++;
        }
    }
}
