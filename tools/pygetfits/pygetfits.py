#!/usr/bin/env python

import sys
import pyfits
import numpy as np

if __name__ == "__main__":
    if len(sys.argv) != 7:
        print "%s <input> <output> <xcenter> <ycenter> <width> <height>"
        print "\tExtract the <width>*<height> subimage centered at (<xcenter>, <ycenter>)"
        print "\tfrom <input> and write it to <output>. Fill it with NaN when the"
        print "\tcenter is to close to an edge"
        sys.exit(1)
    inFitsName = sys.argv[1]
    outFitsName = sys.argv[2]
    xCenter = int(sys.argv[3])
    yCenter = int(sys.argv[4])
    dx = int(sys.argv[5])
    dy = int(sys.argv[6])
    inFits = pyfits.open(inFitsName)
    image = inFits[0].data
    (height, width) = image.shape
    # print height, width
    xLeft = xCenter-dx/2
    if xLeft<0:
        xImageLeft = 0
    else:
        xImageLeft = xLeft
    xRight = xCenter+dx/2
    if xRight>width:
        xImageRight = width
    else:
        xImageRight = xRight
    yUp = yCenter-dy/2
    if yUp<0:
        yImageUp = 0
    else:
        yImageUp = yUp
    yDown = yCenter+dy/2
    if yDown>height:
        yImageDown = height
    else:
        yImageDown = yDown
    
    tmpsubimage = image[ yImageUp:yImageDown, xImageLeft:xImageRight]
    subimage = np.ndarray(shape = (dy, dx))
    nan = float('NaN')
    subimage.fill(nan)
    tmpheight, tmpwidth = tmpsubimage.shape
    # print "tmp = ", tmpheight, tmpwidth
    # print "target = ", -yUp+tmpheight+yUp, -xLeft+tmpwidth+xLeft
    # print yUp
    subimage[yImageUp-yUp:yImageUp-yUp+tmpheight, xImageLeft-xLeft:xImageLeft-xLeft+tmpwidth ] = tmpsubimage[:,:]

    inFits[0].data = subimage
    inFits.writeto(outFitsName, clobber = True)
