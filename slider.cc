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


X_slider::X_slider (X_window *parent, X_callback *callb, X_slider_style *style, X_scale_style *scale,
		    int xp, int yp, int xs, int ys, int cbid) :
    X_window (parent, xp, yp, xs, ys, style->bg),
    _callb (callb),
    _style (style),
    _scale (scale),
    _x (0),
    _y (0),
    _i (_scale->pix [0]),
    _d (9999),
    _cbid (cbid),
    _knob (_style->knob),
    _mark (_style->mark)
{
    x_add_events (ExposureMask | Button1MotionMask | ButtonPressMask | ButtonReleaseMask);
}


void X_slider::brelease (XButtonEvent *E)
{
    _d = 9999;
    if (_callb) _callb->handle_callb (X_callback::SLIDER | STOP, this, (XEvent *) E);
}





X_hslider::X_hslider (X_window *parent, X_callback *callb, X_slider_style *style, X_scale_style *scale, 
		      int xp, int yp, int h, int cbid) :
    X_slider (parent, callb, style, scale, xp, yp, scale->length (), h, cbid)
{
    _y = h;
    _x = _scale->marg;
}


void X_hslider::handle_event (XEvent *E)
{
    XMotionEvent *M = (XMotionEvent *) E;
    XButtonEvent *B = (XButtonEvent *) E;

    switch (E->type)
    {
    case Expose:
	plines ();
	plknob (_i);
	break;  
 
    case ButtonPress:
	if (B->button == Button4)
	{
             if (domove (_scale->limit (_i + 1)) && _callb)
	     {
	         _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	     }
	}
	else if (B->button == Button5)
	{
             if (domove (_scale->limit (_i - 1)) && _callb)
	     {
	         _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	     }
	}
	else
	{
	    _d = B->x - _x - _i;
	    if (abs (2 * _d) >= _style->h) _d = 9999;
	}
	break;  

    case ButtonRelease:
	brelease (B);
	break;

    case MotionNotify:
	if ((_d < 9999) && domove (_scale->limit (M->x - _x - _d)) && _callb)
	{
	    _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	}

	break;

    default: 
	fprintf (stderr, "X_slider: event %d\n", E->type );
    }
}


void X_hslider::set_color (unsigned long knob, unsigned long mark)
{
    if ((_knob != knob) || (_mark != mark))
    {
        _knob = knob;
        _mark = mark;
        plknob (_i);
    }  
}


void X_hslider::set_val (float v)
{
    domove (_scale->calcpix (v)); 
}


bool  X_hslider::domove (int i)
{
    if (i == _i) return false;
    erase (_i); 
    plines ();
    plknob (_i = i);
    return true;
}


void X_hslider::plines (void)
{
    int x, k, *p;
    GC gc = disp ()->dgc ();

    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
    XSetForeground (dpy (), gc, _scale->fg->pixel);
    k = _scale->nseg + 1;
    p = _scale->pix;  
    while (k--)
    {
        x = _x + *p++;
        XDrawLine (dpy (), win (), gc, x, 0, x, _y);
    }     
    k = _y / 2;
    XSetForeground (dpy (), gc, _style->dark);
    XDrawLine (dpy (), win (), gc, _x, k - 1, _x + _scale->pixlen (), k - 1);
    XSetForeground (dpy (), gc, _style->lite);
    XDrawLine (dpy (), win (), gc, _x, k, _x + _scale->pixlen (), k);
}


void X_hslider::plknob (int i)
{
    int x, y, h, w;
    GC gc = disp ()->dgc ();

    h = _style->h;
    w = _style->w;
    x = i + _x - h / 2;
    y = (_y - w) / 2; 
    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
    XSetForeground (dpy (), gc, _knob);
    XFillRectangle (dpy (), win (), gc, x, y, h, w);
    XSetForeground (dpy (), gc, _mark);
    XDrawLine (dpy (), win (), gc, x + h / 2, y, x + h / 2, y + w);

    XSetForeground (dpy (), gc, _style->lite);
    XDrawLine (dpy (), win (), gc, x - 1, y - 1, x - 1, y + w);
    XDrawLine (dpy (), win (), gc, x - 1, y - 1, x + h, y - 1);

    XSetForeground (dpy (), gc, _style->dark);
    XDrawLine (dpy (), win (), gc, x + h, y + w, x + h, y);
    XDrawLine (dpy (), win (), gc, x + h, y + w, x, y + w);
}


void X_hslider::erase (int i)
{
    int x, y, h, w;
    GC  gc = disp ()->dgc ();

    h = _style->h;
    w = _style->w;
    x = i + _x - h / 2;
    y = (_y - w) / 2; 
    XSetFunction (dpy (), gc, GXcopy);
    XSetForeground (dpy (), gc, _style->bg);
    XFillRectangle (dpy (), win (), gc, x - 1, y - 1, h + 2, w + 2);
}



X_vslider::X_vslider (X_window *parent, X_callback *callb, X_slider_style *style, X_scale_style *scale, 
		      int xp, int yp, int w, int cbid) :
    X_slider (parent, callb, style, scale, xp, yp, w, scale->length (), cbid)
{
    _x = w;
    _y = _scale->marg + scale->pixlen () - 1;
}


void X_vslider::handle_event (XEvent *E)
{
    XMotionEvent *M = (XMotionEvent *) E;
    XButtonEvent *B = (XButtonEvent *) E;

    switch (E->type)
    {
    case Expose:
	plines ();
	plknob (_i);
	break;  
 
    case ButtonPress:
	if (B->button == Button4)
	{
             if (domove (_scale->limit (_i + 1)) && _callb)
	     {
	         _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	     }
	}
	else if (B->button == Button5)
	{
             if (domove (_scale->limit (_i - 1)) && _callb)
	     {
	         _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	     }
	}
	else
	{
	    _d = M->y - _y + _i;
	    if (abs (2 * _d) >= _style->h) _d = 9999;
	}
	break;  

    case ButtonRelease:
	brelease (B);
	break;

    case MotionNotify:
	if ((_d < 9999) && domove (_scale->limit (_d + _y - M->y)) && _callb)
	{
	    _callb->handle_callb (X_callback::SLIDER | MOVE, this, (XEvent *) E);
	}
	break;

    default: 
	fprintf (stderr, "X_slider: event %d\n", E->type );
    }
}


void X_vslider::set_color (unsigned long knob, unsigned long mark)
{
    if ((_knob != knob) || (_mark != mark))
    {
        _knob = knob;
        _mark = mark;
        plknob (_i);
    }  
}


void X_vslider::set_val (float v)
{
    domove (_scale->calcpix (v)); 
}


bool X_vslider::domove (int i)
{
    if (i == _i) return false;
    erase (_i); 
    plines ();
    plknob (_i = i);
    return true;
}


void X_vslider::plines (void)
{
    int k, y, *p;

    GC gc = disp ()->dgc ();

    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
    XSetForeground (dpy (), gc, _scale->fg->pixel);
    k = _scale->nseg + 1;
    p = _scale->pix;  
    while (k--)
    {
        y = _y - *p++;
        XDrawLine (dpy (), win (), gc, 0, y, _x, y);
    }     
    k = _x / 2;
    XSetForeground (dpy (), gc, _style->dark);
    XDrawLine (dpy (), win (), gc, k - 1, _y - _scale->pixlen (), k - 1, _y);
    XSetForeground (dpy (), gc, _style->lite);
    XDrawLine (dpy (), win (), gc, k, _y - _scale->pixlen (), k, _y);
}


void X_vslider::plknob (int i)
{
    int x, y, h, w;
    GC gc = disp ()->dgc ();

    h = _style->h;
    w = _style->w;
    x = (_x - w) / 2; 
    y = _y - i - h / 2;

    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
    XSetForeground (dpy (), gc, _knob);
    XFillRectangle (dpy (), win (), gc, x, y, w, h);
    XSetForeground (dpy (), gc, _mark);
    XDrawLine (dpy (), win (), gc, x, y + h / 2, x + w, y + h / 2);

    XSetForeground (dpy (), gc, _style->lite);
    XDrawLine (dpy (), win (), gc, x - 1, y - 1, x - 1, y + h);
    XDrawLine (dpy (), win (), gc, x - 1, y - 1, x + w, y - 1);
    XSetForeground (dpy (), gc, _style->dark);
    XDrawLine (dpy (), win (), gc, x + w, y + h, x, y + h);
    XDrawLine (dpy (), win (), gc, x + w, y + h, x + w, y);
}


void X_vslider::erase (int i)
{
    int x, y, h, w;
    GC  gc = disp ()->dgc ();

    h = _style->h;
    w = _style->w;
    x = (_x - w) / 2; 
    y = _y - i - h / 2;
    XSetFunction (dpy (), gc, GXcopy);
    XSetForeground (dpy (), gc, _style->bg);
    XFillRectangle (dpy (), win (), gc, x - 1, y - 1, w + 2, h + 2);
}

