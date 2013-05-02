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


X_meter::X_meter (X_window *parent, X_meter_style *style, X_scale_style *scale,
		  int xp, int yp, int xs, int ys) :
    X_window (parent, xp, yp, xs, ys, style->bg),
    _style (style),
    _scale (scale),
    _x (scale->marg),
    _y (scale->marg),
    _d (0)
{
    _d0 = _i  = -1;
    _d1 = _d2 = _scale->pixlen () - 1;
    x_add_events (ExposureMask);
}


void X_meter::handle_event (XEvent *E)
{
    switch (E->type)
    {
    case Expose:
	expose ((XExposeEvent *) E);
	break;  
    }
}


void X_meter::expose (XExposeEvent *)
{
    XClearWindow (dpy (), win ());
    XSetLineAttributes (dpy (), dgc (), 0, LineSolid, CapButt, JoinBevel);
    plotsect (_d0, _i, _d0, _d1, _style->act.c0, _style->off.c0);
    plotsect (_d1, _i, _d0, _d1, _style->act.c0, _style->off.c0);
    plotsect (_d1, _i, _d1, _d2, _style->act.c1, _style->off.c1);
    plotsect (_d2, _i, _d1, _d2, _style->act.c1, _style->off.c1);
}


void X_meter::set_val (float v)
{
    int i = _scale->calcpix (v);

    if (i != _i)
    {
	plotsect (_i, i, _d0, _d1, _style->act.c0, _style->off.c0);
	plotsect (_i, i, _d1, _d2, _style->act.c1, _style->off.c1);
	_i = i;
    }
}


void X_meter::set_ref (float v)
{
    int t = _scale->calcpix (v) - 1;

    if (t < _d0) t = _d0;
    if (t != _d1)
    {
	plotsect (_d1, t, _d0, _i, _style->act.c0, _style->act.c1);
	plotsect (_d1, t, _i, _d2, _style->off.c0, _style->off.c1);
	_d1 = t;
    }
}


void X_meter::plotsect (int i0, int i1, int imin, int imax, int cl, int cr)
{
    int a, b;

    a = (i0 < imin) ? imin : (i0 > imax) ? imax : i0;     
    b = (i1 < imin) ? imin : (i1 > imax) ? imax : i1;     

    if (a != b)
    {
	GC gc = disp ()->dgc ();
	XSetFunction (dpy (), gc, GXcopy);
	XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);

	if (a < b)
	{
	    XSetForeground (dpy (), gc, cl);
	    psect (gc, b, a);
	    plotmarks (a, b, cl);
	}
	else
	{
	    XSetForeground (dpy (), gc, cr);
	    psect (gc, a, b);
	    plotmarks (b, a, cr);
	}
    }
}


void X_meter::plotmarks (int a, int b, unsigned long c)
{
    int  i, k, *p;

    k = _scale->nseg + 1;
    p = _scale->pix;  
    if (c == _style->act.c0 || c == _style->act.c1) XSetForeground (dpy (), dgc (), _style->act.mk);
    else                                            XSetForeground (dpy (), dgc (), _style->off.mk);
    while (k--)
    {
	i = *p++;
	if (i > b) break;
	if (i > a) pmark (dgc (), i);
    }     
}




X_hmeter::X_hmeter (X_window *parent, X_meter_style *style, X_scale_style *scale, 
		    int xp, int yp, int h) :
    X_meter (parent, style, scale, xp, yp, scale->length (), h)
{
    _d = h - 2 * scale->marg;
}


void X_hmeter::psect (GC g, int a, int b)
{
    XFillRectangle (dpy (), win (), g, _x + b + 1, _y, a - b, _d);
}


void X_hmeter::pmark (GC g, int a)
{
    XDrawLine (dpy (), win (), g, _x + a, _y + 1, _x + a, _y + _d);       
}




X_vmeter::X_vmeter (X_window *parent, X_meter_style *style, X_scale_style *scale, 
		    int xp, int yp, int w) :
    X_meter (parent, style, scale, xp, yp, w, scale->length ())
{
    _d = w - 2 * scale->marg;
    _y += scale->pixlen () - 1;
}


void X_vmeter::psect (GC g, int a, int b)
{
    XFillRectangle (dpy (), win (), g, _x, _y - a, _d, a - b);
}


void X_vmeter::pmark (GC g, int a)
{
    XDrawLine (dpy (), win (), g, _x + 1, _y - a, _x + _d, _y - a);
}


