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


#include "clxclient.h"


const char *clxclient_version (void) { return VERSION; }


X_display::X_display (const char *name) :
    _dsn (0), _dcm (0), _dgc (0), _dvi (0), _xft (0), _xim (0)
{
    int      i;
    XImage  *I;

    _dpy = XOpenDisplay (name); 
    if (! _dpy) return;

    _dsn = DefaultScreen (_dpy);
    _dcm = DefaultColormap (_dpy, _dsn);
    _dgc = DefaultGC (_dpy, _dsn);
    _dvi = DefaultVisual (_dpy, _dsn);
    _xim = XOpenIM (_dpy, 0, 0, 0);

    for (i = 0; i < N_IMG1515; i++)
    {
	I = XCreateImage (_dpy, _dvi, 1, XYBitmap, 0, _imgdef1515 + 30 * i, 15, 15, 8, 2);
	I->bitmap_unit = 8;
	I->bitmap_pad  = 8;
	I->bitmap_bit_order = LSBFirst;
	XInitImage (I);
	_imgptr1515 [i] = I;
    }
}


X_display::~X_display (void)
{
    int i;

    for (i = 0; i < N_IMG1515; i++)
    {  
        _imgptr1515 [i]->data = 0;
        XDestroyImage (_imgptr1515 [i]);
    }
    if (_dpy) XCloseDisplay (_dpy); 
}


unsigned long X_display::alloc_color (const char *name, unsigned long d)
{
    XColor C;

    if (XParseColor (_dpy, _dcm, name, &C))
    {
	if (XAllocColor (_dpy, _dcm, &C)) return C.pixel;
	else fprintf (stderr, "-- Can't alloc '%s', using default (%ld).\n", name, d);
    }
    else fprintf (stderr, "-- Can't parse '%s', using default (%ld).\n", name, d);
    return d;
}



unsigned long X_display::alloc_color (float r, float g, float b)
{
    XColor C;

    C.red   = (int)(65535 * r);
    C.green = (int)(65535 * g);
    C.blue  = (int)(65535 * b);

    if (XAllocColor (_dpy, _dcm, &C)) return C.pixel;
    else return 0;
}



XftColor *X_display::alloc_xftcolor (const char *name, XftColor *D)
{
    XftColor *C = new XftColor;

    if (! XftColorAllocName (_dpy, _dvi, _dcm, name, C))
    {
         if (D)
	 {
             XftColorAllocValue (_dpy, _dvi, _dcm, &(D->color), C);
             fprintf (stderr, " -- Can't alloc '%s', using default (%ld).\n", name, D->pixel); 
	 }
         else fprintf (stderr, " -- Can't alloc '%s', no default provided.\n", name);
    }
    return C;
}    


XftColor *X_display::alloc_xftcolor (float r, float g, float b, float a)
{
    XftColor     *C = new XftColor;
    XRenderColor  R; 

    R.red   = (int)(65535 * r);
    R.green = (int)(65535 * g);
    R.blue  = (int)(65535 * b);
    R.alpha = (int)(65535 * a);

    XftColorAllocValue (_dpy, _dvi, _dcm, &R, C);
    return C;
}    


void X_display::free_xftcolor (XftColor *C)
{
    XftColorFree (_dpy, _dvi, _dcm, C);
    delete C;
}


#define FALLBACK "Fixed"


XFontStruct *X_display::alloc_font (const char *name)
{
    XFontStruct *F = NULL;

    if (name)
    {
	if (! (F = XLoadQueryFont (_dpy, name)))
	{
	    fprintf (stderr, "-- Can't load font '%s'\n", name);
	    fprintf (stderr, "-- Trying to use %s instead.\n", FALLBACK);
	}
    }
    if (! F)
    {
	if (! (F = XLoadQueryFont (_dpy, FALLBACK)))
	{
	    fprintf(stderr, "-- Can't load font %s \n", FALLBACK);
	    fprintf(stderr, "-- No useable font - X11 aborted.\n");
	    exit (1);
	}
    }
    return F; 
}


XftFont *X_display::alloc_xftfont (const char *name)
{
    return XftFontOpenName (_dpy, _dsn, name);
}


void X_display::free_xftfont (XftFont *F)
{
    XftFontClose (_dpy, F);
}


XImage *X_display::image1515 (unsigned int i)
{
    return (i < N_IMG1515) ? _imgptr1515 [i] : 0;
}


char X_display::_imgdef1515 [N_IMG1515 * 30] =
{
    0,0,0,0,0,0,0,2,0,3,128,3,192,3,224,3,192,3,128,3,0,3,0,2,0,0,0,0,0,0, 
    0,0,0,0,0,0,32,0,96,0,224,0,224,1,224,3,224,1,224,0,96,0,32,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,128,0,192,1,224,3,240,7,248,15,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,248,15,240,7,224,3,192,1,128,0,0,0,0,0,0,0,0,0,0,0, 
    0,0,0,0,0,0,192,1,192,1,192,1,248,15,248,15,248,15,192,1,192,1,192,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,248,15,248,15,248,15,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,248,15,248,15,24,12,24,12,24,12,24,12,24,12,248,15,248,15,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,224,3,224,3,224,3,224,3,224,3,0,0,0,0,0,0,0,0,0,0
};






