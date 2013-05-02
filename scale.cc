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


int X_scale_style::calcpix (float v)
{
    int  k, *P, p0, p1;
    float *V, v0, v1;

    k = nseg;
    P = pix;  
    V = val;

    p0 = *P++;
    v0 = *V++; 
    if (v < v0) return p0;
    while (k--)
    {
	p1 = *P++;
	v1 = *V++;
	if (v > v1)
	{
	    p0 = p1;
	    v0 = v1;   
	}
	else return (int)(p0 + (p1 - p0) * (v - v0) / (v1 - v0) + 0.5);
    }
    return p0;
}


float X_scale_style::calcval (int p)
{
    int  k, *P, p0, p1;
    float *V, v0, v1;

    k = nseg;
    P = pix;  
    V = val;

    p0 = *P++;
    v0 = *V++; 
    if (p < p0) return v0;
    while (k--)
    {
	p1 = *P++;
	v1 = *V++;
	if (p > p1)
	{
	    p0 = p1;
	    v0 = v1;   
	}
	else return v0 + (v1 - v0) * (p - p0) / (p1 - p0);
    }
    return v0; 
}


int X_scale_style::limit (int i)
{
    if (i < pix [0]) return pix [0]; 
    if (i > pix [nseg]) return pix [nseg]; 
    return i;
}



X_hscale::X_hscale (X_window  *parent, X_scale_style *style, int xp, int yp, int h, int d) :
    X_window (parent, xp, yp, style->length (), h, style->bg), 
    _style (style),
    _h (h),
    _d (d)
{
    x_add_events (ExposureMask);
}


void X_hscale::handle_event (XEvent *E)
{
    switch (E->type)
    {
    case Expose:
	expose ((XExposeEvent *) E);
	break;  
    }
}


void X_hscale::expose (XExposeEvent *)
{
    int            k, l, m, x, y, *p;
    const char   **t;
    XftColor      *C = _style->fg;
    XftFont       *F = _style->font;
    XGlyphInfo     K;

    XClearWindow (dpy (), win ());
    XftDrawChange (xft (), win ());

    k = _style->nseg + 1;
    p = _style->pix;  
    t = _style->text;
    m = _style->marg;
    y = (_h + F->ascent - F->descent) / 2;
    while (k--)
    {
	if (*t)
	{
	    l = strlen (*t);  
            XftTextExtentsUtf8 (dpy (), F, (const FcChar8 *) *t, l, &K);
            x = m + *p - K.width / 2;
            XftDrawStringUtf8 (xft (), C, F, x, y, (const FcChar8 *) *t, l);
	}
	p++;
	t++;
    }
}



X_vscale::X_vscale (X_window *parent, X_scale_style *style, int xp, int yp, int w, int d) :
    X_window (parent, xp, yp, w, style->length (), style->bg), 
    _style (style),
    _w (w),
    _d (d)
{
    x_add_events (ExposureMask);
}


void X_vscale::handle_event (XEvent *E)
{
    switch (E->type)
    {
    case Expose:
	expose ((XExposeEvent *) E);
	break;  
    }
}


void X_vscale::expose (XExposeEvent *)
{
    int            k, l, m, x, y, *p;
    const char   **t;
    XftColor      *C = _style->fg;
    XftFont       *F = _style->font;
    XGlyphInfo     K;

    XClearWindow (dpy (), win ());
    XftDrawChange (xft (), win ());

    k = _style->nseg + 1;
    p = _style->pix;  
    t = _style->text;
    m = _style->marg + _style->pixlen () - 1 + F->ascent / 2;
    while (k--)
    {
	if (*t)
	{
	    l = strlen (*t);  
	    y = m - *p;
	    if (_d <= 0)
	    {
                XftTextExtentsUtf8 (dpy (), F, (const FcChar8 *) *t, l, &K);
		if (_d) x = _w  + _d - K.width;
		else    x = (_w - K.width) / 2;
	    }
	    else x = _d; 
            XftDrawStringUtf8 (xft (), C, F, x, y, (const FcChar8 *) *t, l);
	}
	p++;
	t++;
    }
}

