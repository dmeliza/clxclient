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


#define DL 17
#define DW 13


X_scroll::X_scroll (X_window       *parent,
                    X_callback     *callb,
                    X_button_style *style,
                    int xp,
                    int yp,
                    int xs,
                    int ys,
                    int opts) :
    X_window (parent, xp, yp, xs, ys, style->color.shadow.bgnd),
    _style (style),
    _callb (callb),
    _xs (xs),
    _ys (ys),
    _km (0),
    _k0 (0),
    _dk (0),
    _dw (0),
    _zz (INT_MIN),
    _offs (0.0f),
    _frac (1.0f)
{
    X_ibutton *B;

    x_add_events (ExposureMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
    style->type = X_button_style::RAISED;

    if (xs >= 2 * ys)
    {
        style->size.x = DL;
        style->size.y = DW;
	_km = xs - 1;
        _dw = (ys - DW) >> 1;
        if (opts & ENDGAP) _km -= _ys;
        if (opts & PLUSMIN)
	{
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _km, _dw, disp ()->image1515 (X_display::IMG_MI), X_callback::SCROLL | B1MI); 
            B->x_set_win_gravity (NorthEastGravity);
            B->x_map ();
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _km, _dw, disp ()->image1515 (X_display::IMG_PL), X_callback::SCROLL | B1PL); 
            B->x_set_win_gravity (NorthEastGravity);
            B->x_map ();
	}
        if (opts & MIDGAP) _km -= _ys;
        if (opts & ARROWS)
	{
            _km -= DL;      
            B = new X_ibutton (this, this, style, _km, _dw, disp ()->image1515 (X_display::IMG_RT), X_callback::SCROLL | B1RD); 
            B->x_set_win_gravity (NorthEastGravity);
            B->x_map ();
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _km, _dw, disp ()->image1515 (X_display::IMG_LT), X_callback::SCROLL | B1LU); 
            B->x_set_win_gravity (NorthEastGravity);
            B->x_map ();
	}
    }
    else if (ys >= 2 * xs)
    {
        style->size.x = DW;
        style->size.y = DL;
	_km = ys - 1;
        _dw = (xs - DW) >> 1;
        if (opts & ENDGAP) _km -= _xs;
        if (opts & PLUSMIN)
	{
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _dw, _km, disp ()->image1515 (X_display::IMG_MI), X_callback::SCROLL | B1MI); 
            B->x_set_win_gravity (SouthWestGravity);
            B->x_map ();
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _dw, _km, disp ()->image1515 (X_display::IMG_PL), X_callback::SCROLL | B1PL); 
            B->x_set_win_gravity (SouthWestGravity);
            B->x_map ();
	}
        if (opts & MIDGAP) _km -= _xs;
        if (opts & ARROWS)       
	{
            _km -= DL;      
            B = new X_ibutton (this, this, style, _dw, _km, disp ()->image1515 (X_display::IMG_DN), X_callback::SCROLL | B1RD); 
            B->x_set_win_gravity (SouthWestGravity);
            B->x_map ();
 	    _km -= DL;
            B = new X_ibutton (this, this, style, _dw, _km, disp ()->image1515 (X_display::IMG_UP), X_callback::SCROLL | B1LU); 
            B->x_set_win_gravity (SouthWestGravity);
            B->x_map ();
	}
    }
    _km -= 1;
}


X_scroll::~X_scroll (void)
{
}


void X_scroll::handle_event (XEvent *E)
{
    switch (E->type)
    {
    case Expose:
	expose ((XExposeEvent *) E);
	break;  
 
    case ButtonPress:
	bpress ((XButtonEvent *) E);
	break;  

    case ButtonRelease:
	brelease ((XButtonEvent *) E);
	break;

    case MotionNotify:
        bmotion ((XMotionEvent *) E);
	break;
    }
}


void X_scroll::bpress (XButtonEvent *E)
{
    int k;

    k = (_xs > _ys) ? E->x - 1 : E->y - 1;
    if (! _callb || (k < 0) || (k >= _km))  return; 
    k -= _k0;
    if   (k >= _dk) _callb->handle_callb (X_callback::SCROLL | ((E->button == Button3) ? C3RD : C1RD), this, 0);
    else if (k < 0) _callb->handle_callb (X_callback::SCROLL | ((E->button == Button3) ? C3LU : C1LU), this, 0);
    else _zz = k + _k0;
}


void X_scroll::brelease (XButtonEvent *E)
{
    if (_zz != INT_MIN)
    {
	_zz = INT_MIN;
        _callb->handle_callb (X_callback::SCROLL | STOP, this, 0);
    }
}


void X_scroll::bmotion (XMotionEvent *E)
{
    int d, k;

    if (_zz == INT_MIN) return;
    k = (_xs > _ys) ? E->x - 1 : E->y - 1;
    d = k - _zz;
    if (_k0 + d <= 0)
    {
        d = -_k0;
        _offs = 0.0f;
    }
    else if (_k0 + d >= _km - _dk)
    {
        d = _km - _dk - _k0;
	_offs = 1.0f;
    }
    else
    {
        _offs += d / (float)(_km - _dk);
    }
    if (d)
    {
	_zz += d;
	_callb->handle_callb (X_callback::SCROLL | MOVE, this, 0);
        redraw ();
    }
}


void X_scroll::expose (XExposeEvent *E)
{
   if (E->count == 0) redraw ();
}


void X_scroll::geom (float offs, float frac, int size)
{
    if (size)
    {
        if (_xs > _ys)
        { 
	    _km += size - _xs;
            _xs = size;
	}
        else
	{
           _km += size - _ys; 
           _ys = size;
	}
        XResizeWindow (dpy (), win (), _xs, _ys); 
    }
    _offs = offs;
    _frac = frac;
    redraw ();
}


void X_scroll::move (float drel)
{
    _offs += drel * _frac;
    if (_offs < 0.0f) _offs = 0.0f;
    if (_offs > 1.0f) _offs = 1.0f;
    redraw ();
}


void X_scroll::redraw (void)
{
    _dk = (int)(_frac * _km + 0.5f);    
    if (_dk < 8) _dk = 8;
    _k0 = (int)(_offs * (_km - _dk) + 0.5f);
    XSetFunction (dpy (), dgc (), GXcopy);
    XSetLineAttributes (dpy (), dgc (), 0, LineSolid, CapButt, JoinBevel);
    if (_xs > _ys)
    {
	XSetForeground (dpy (), dgc (), _style->color.shadow.bgnd);
        XFillRectangle (dpy (), win (), dgc (), 0, 0, _k0 + 2, _ys);
        XFillRectangle (dpy (), win (), dgc (), _k0 + _dk, 0, _km - _k0 - _dk + 1, _ys);
	XSetForeground (dpy (), dgc (), _style->color.bg [1]);
        XFillRectangle (dpy (), win (), dgc (), _k0 + 2, _dw + 1, _dk - 2, 11);
	XSetForeground (dpy (), dgc (), _style->color.shadow.lite);
	XDrawLine (dpy (), win (), dgc (), _k0 + 1, _dw, _k0 + 1, _dw + DW - 2);
	XDrawLine (dpy (), win (), dgc (), _k0 + 1, _dw, _k0 + _dk - 1, _dw);
	XSetForeground (dpy (), dgc (), _style->color.shadow.dark);
	XDrawLine (dpy (), win (), dgc (), _k0 + _dk, _dw + DW - 1, _k0 + 2, _dw + DW - 1);
	XDrawLine (dpy (), win (), dgc (), _k0 + _dk, _dw + DW - 1, _k0 + _dk, _dw + 1);
    }
    else
    {
	XSetForeground (dpy (), dgc (), _style->color.shadow.bgnd);
        XFillRectangle (dpy (), win (), dgc (), 0, 0, _xs, _k0 + 2);
        XFillRectangle (dpy (), win (), dgc (), 0, _k0 + _dk, _xs, _km - _k0 - _dk + 1);
	XSetForeground (dpy (), dgc (), _style->color.bg [1]);
        XFillRectangle (dpy (), win (), dgc (), _dw + 1, _k0 + 2, 11, _dk - 2);
	XSetForeground (dpy (), dgc (), _style->color.shadow.lite);
	XDrawLine (dpy (), win (), dgc (), _dw, _k0 + 1, _dw + DW - 2, _k0 + 1);
	XDrawLine (dpy (), win (), dgc (), _dw, _k0 + 1, _dw, _k0 + _dk - 1);
	XSetForeground (dpy (), dgc (), _style->color.shadow.dark);
	XDrawLine (dpy (), win (), dgc (), _dw + DW - 1, _k0 + _dk, _dw + DW - 1, _k0 + 2);
	XDrawLine (dpy (), win (), dgc (), _dw + DW - 1, _k0 + _dk, _dw + 1, _k0 + _dk);
    }
}


void X_scroll::handle_callb (int type, X_window *W, XEvent *E)
{
    X_button     *B = (X_button *) W;
    XButtonEvent *Z = (XButtonEvent *) E;
    if (_callb) _callb->handle_callb (B->cbid () + ((Z->button == Button3) ? 1 : 0), this, E);
}
