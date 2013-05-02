// ---------------------------------------------------------------------------------
//
//  Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// ---------------------------------------------------------------------------------


#include <string.h>
#include "clxclient.h"


X_draw::X_draw (Display *dpy, Drawable drw, GC gct, XftDraw *xft) :
    _dpy (dpy), _drw (drw), _gct (gct), 
    _xft_draw (xft), _xft_color (0), _xft_font (0), 
    _xx (0), _yy (0)
{
    if (_xft_draw && XftDrawDrawable (_xft_draw) != _drw) XftDrawChange (_xft_draw, _drw);
}


int X_draw::textwidth (const char *str)
{
    int len;
    XGlyphInfo G;

    if (str && (len = strlen (str)))
    {
	if (_xft_font)
	{
	    XftTextExtentsUtf8 (_dpy, _xft_font, (const FcChar8 *) str, len, &G);
	    return G.width;
	}
	else if (_x11_font) return XTextWidth (_x11_font, str, len);
    }
    return 0;
}


void X_draw::drawstring (const char *str, int xal)
{
    int len, dx = 0;
    XGlyphInfo G;

    if (str && (len = strlen (str)))
    {
	if (_xft_font)
	{
	    if (xal >= 0)
	    {
		XftTextExtentsUtf8 (_dpy, _xft_font, (const FcChar8 *) str, len, &G);
		dx = G.width;
		if (xal == 0) dx /= 2;
	    }
	    XftDrawStringUtf8 (_xft_draw, _xft_color, _xft_font, _xx - dx, _yy, (const FcChar8 *) str, len);
	}
	else if (_x11_font)
	{
	    if (xal >= 0)
	    {
		dx = XTextWidth (_x11_font, str, len);
		if (xal == 0) dx /= 2;
	    }
	    XDrawString (_dpy, _drw, _gct, _xx - dx,  _yy, str, len);
	}
    }
}


void X_draw::drawpoints (int n, XPoint *P)
{
   XDrawPoints (_dpy, _drw, _gct, P, n, CoordModeOrigin);
}


void X_draw::drawlines (int n, XPoint *P)
{
    int k, m;

    m = (XMaxRequestSize (_dpy) - 3) / 2;
    while (n > 1)
    {
	if (n > m) k = m;
	else       k = n;
	XDrawLines (_dpy, _drw, _gct, P, k, CoordModeOrigin);
        P += k - 1;
	n -= k - 1;
    }
}


void X_draw::drawsegments (int n, XSegment *S)
{
    XDrawSegments (_dpy, _drw, _gct, S, n);
}


void X_draw::setclip (int x0, int y0, int x1, int y1)
{
    XRectangle R;

    R.x = x0;
    R.y = y0;
    R.width  = x1 - x0;
    R.height = y1 - y0;
    XSetClipRectangles (_dpy, _gct, 0, 0, &R, 1, Unsorted);
}


void X_draw::movepix (int dx, int dy, int xs, int ys)
{
    XGCValues G;

    G.graphics_exposures = True;
    G.function = GXcopy;
    XChangeGC (_dpy, _gct, GCGraphicsExposures | GCFunction, &G);
    XCopyArea (_dpy, _drw, _drw, _gct, dx, dy, xs, ys, 0, 0);
    G.graphics_exposures = False;
    XChangeGC (_dpy, _gct, GCGraphicsExposures, &G);
}




