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


X_menuwin::X_menuwin (X_window        *parent,
                      X_callback      *callb,
                      X_menuwin_style *style,
                      int xp,
                      int yp,
		      X_menuwin_item  *items) :
    X_window (parent, xp, yp, 100, 100, style->color.bg->pixel),
    _style (style),
    _callb (callb),
    _items (items),
    _xs (10),
    _ys (10),
    _isel (-1),
    _open (0)
{
    int             i, k, ys, xm;
    X_menuwin_item *P;
    XGlyphInfo      G;    

    xm = 0; 
    ys = 1;  
    i = 0;
    P = items;
    while (i < MAXITEM)   
    {
	if (P->_bits & X_menuwin_item::SPACE) ys += _style->step / 2;
	_ypos [i] = ys;
	_tlen [i] = k = strlen (P->_text);
	XftTextExtentsUtf8 (dpy (), _style->font, (const FcChar8 *)(P->_text), k, &G);
	if (xm < G.width) xm = G.width;
	i++;
	ys += _style->step;
	if (P->_bits & X_menuwin_item::TITLE) ys++;
	if (P->_bits & X_menuwin_item::LAST) break;
	P++;
    }
     
    _nit = i;
    _xs = xm + DL + DR;
    _ys = ys + 2;

    if (_style->type & X_menuwin_style::PDOWN)
    {
	x_resize (_xs, _style->step);
	x_map ();
    }
    else x_resize (_xs, _ys);

    x_add_events (ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask);
}


X_menuwin::~X_menuwin (void)
{
}


void X_menuwin::show (void)
{
    if (_style->type & X_menuwin_style::PDOWN) x_resize (_xs, _ys);
    x_mapraised ();
    _open = true;
    _isel = -1;
}


void X_menuwin::hide (void)
{
    if (_style->type & X_menuwin_style::PDOWN) x_resize (_xs, _style->step);
    else x_unmap ();
    _open = false;
}


void X_menuwin::handle_event (XEvent *E)
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
	brelse ((XButtonEvent *) E);
	break;  

    case EnterNotify:
	enter ((XEnterWindowEvent *) E);
	break;

    case LeaveNotify:
	leave ((XLeaveWindowEvent *) E);
	break;

    case MotionNotify:
	motion ((XPointerMovedEvent *) E);
	break;
    }
}


void X_menuwin::bpress (XButtonEvent *E)
{
    if (_open)
    {
	if (_isel >= 0) _callb->handle_callb (X_callback::MENU | SEL, this, (XEvent *) E);
	else            _callb->handle_callb (X_callback::MENU | CAN, this, (XEvent *) E);
    }
    else _callb->handle_callb (X_callback::MENU | OPEN, this, (XEvent *) E);
}


void X_menuwin::brelse (XButtonEvent *E)
{
    if (_open)
    {
	if (_isel >= 0) _callb->handle_callb (X_callback::MENU | SEL, this, (XEvent *) E);
	else            _callb->handle_callb (X_callback::MENU | CAN, this, (XEvent *) E);
    }
}


void X_menuwin::expose (XExposeEvent * E)
{
    if (E->count == 0) redraw (true);
}


void X_menuwin::enter (XEnterWindowEvent * E)
{
    if (_open) select (findit (E->x, E->y));
}


void X_menuwin::leave (XLeaveWindowEvent * E)
{
    if (_open) select (-1);
}


void X_menuwin::motion (XPointerMovedEvent * E)
{
    if (_open) select (findit (E->x, E->y));
}


void X_menuwin::redraw (bool clear)
{
    int  i, x, y;
    GC gc = disp ()->dgc ();
    const X_menuwin_item *P;

    if (clear) XClearWindow (dpy (), win ());

    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);

    for (i = 0, P = _items; i < _nit; i++, P++)
    {
	y = _ypos [i];
        drawit (i, i == _isel);
	if (! _open) break;
	if (P->_bits & X_menuwin_item::TITLE)
	{
	    y += _style->step;
            XSetForeground (dpy (), dgc (), _style->color.fg [0]->pixel);
	    XDrawLine (dpy (), win (), gc, 1, y, _xs - 2, y);
	}   
    }

    y = _open ? _ys - 1 : _style->step - 1;
    x = _xs - 1;
    switch (_style->type & 255)
    {
    case X_menuwin_style::BORDER:
        XSetForeground (dpy (), gc, _style->color.fg [0]->pixel);
	XDrawRectangle (dpy (), win (), gc, 0, 0, x, y);
	break;

    case X_menuwin_style::RAISED:
	XSetForeground (dpy (), gc, _style->color.shadow.lite);
	XDrawLine (dpy (), win (), gc, 0, 0, 0, y);
	XDrawLine (dpy (), win (), gc, 0, 0, x, 0);
	XSetForeground (dpy (), gc, _style->color.shadow.dark);
	XDrawLine (dpy (), win (), gc, x, y, x, 1);
	XDrawLine (dpy (), win (), gc, x, y, 1, y);
	break;
    }
}


void X_menuwin::drawit (int k, bool inv)
{
    X_menuwin_item  *P = _items + k;
    int              y = _ypos [k];
    int              d = (_style->step + _style->font->ascent - _style->font->descent) / 2;
    XftColor        *bg = _style->color.bg;
    XftColor        *fg = (P->_bits & X_menuwin_item::MASKED) ? _style->color.mm : _style->color.fg [P->_bits & X_menuwin_item::COLMASK];

    if (XftDrawDrawable (xft ()) != win ()) XftDrawChange (xft (), win ());
    XSetFunction (dpy (), dgc (), GXcopy);
    XSetForeground (dpy (), dgc (), (inv) ? fg->pixel : bg->pixel);
    XFillRectangle (dpy (), win (), dgc (), DL - 2, y, _xs - DL - DR + 4, _style->step);
    XftDrawStringUtf8 (xft (), (inv) ? bg : fg , _style->font, DL, y + d, (const FcChar8 *)(P->_text), _tlen [k]);
}


void X_menuwin::select (int k)
{
    if (k != _isel)
    {
	if (_isel >= 0) drawit (_isel, false);
	_isel = k;
	if (_isel >= 0) drawit (_isel, true);
    }
}


int X_menuwin::findit (int x, int y)
{
    int i;

    for (i = 0; i < _nit; i++)
    {
        if (   (y > _ypos [i])
	       && (y < _ypos [i] + _style->step - 1)
	       && (! (_items [i]._bits & (X_menuwin_item::MASKED | X_menuwin_item::TITLE)))) return i;
    }
    return -1;
}


void X_menuwin::set_mask (unsigned m)
{
    for (int i = 0; i < _nit; i++)
    {
	if (m & 1) _items [i]._bits |= X_menuwin_item::MASKED;
	m >>= 1; 
    }
}


void X_menuwin::clr_mask (unsigned m)
{
    for (int i = 0; i < _nit; i++)
    {
	if (m & 1) _items [i]._bits &= ~X_menuwin_item::MASKED;
	m >>= 1; 
    }
}


