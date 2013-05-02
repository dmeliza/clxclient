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


void X_hints::position (int x, int y)
{
    _sh.flags |= PPosition;
    _sh.x = x;
    _sh.y = y;        
}


void X_hints::size (int x, int y)
{
    _sh.flags |= PSize;
    _sh.width  = x;
    _sh.height = y;        
}


void X_hints::minsize (int x, int y)
{
    _sh.flags |= PMinSize;
    _sh.min_width  = x;
    _sh.min_height = y;        
}


void X_hints::maxsize (int x, int y)
{
    _sh.flags |= PMaxSize;
    _sh.max_width  = x;
    _sh.max_height = y;        
}


void X_hints::sizeinc (int x, int y)
{
    _sh.flags |= PResizeInc;
    _sh.width_inc  = x;
    _sh.height_inc = y;        
}


void X_hints::input (int input)
{
    _mh.flags |= InputHint;
    _mh.input = input;
}


void X_hints::state (int state)
{
    _mh.flags |= StateHint;
    _mh.initial_state = state;
}

void X_hints::group (Window group)
{
    _mh.flags |= WindowGroupHint;
    _mh.window_group = group;
}




X_window::X_window (X_display *disp) :
    _ebits (0), _disp (disp), _pwin (0), _next (0), _list (0)
{
    _wind = DefaultRootWindow (disp->_dpy);
}


X_window::X_window (X_window *pwin, int xpos, int ypos, int xsize, int ysize,
		    unsigned long bgcol, unsigned long bdcol, int bdpix) :
    _ebits (0), _disp (pwin->_disp), _pwin (pwin), _next (pwin->_list), _list (0)
{
    _pwin->_list = this;
    _wind = XCreateSimpleWindow (_disp->_dpy, pwin->_wind,
                                 xpos, ypos, xsize, ysize,
                                 bdpix, bdcol, bgcol);
}
 

X_window::~X_window (void)
{
    X_window *T;

    while (_list) delete _list; 
    if (_pwin)
    {
        T = _pwin->_list;
        if (T == this) _pwin->_list = _next;
        else
	{
	    while (T && T->_next != this) T = T->_next;
            if (T) T->_next = _next;
	}
        XDestroyWindow (_disp->_dpy, _wind);
        XFlush (dpy ());
    }
}
		  

/*
X_window::~X_window (void)
{
    X_window *T;

    if (_pwin)
    {
        T = _pwin->_list;
        if (T == this) _pwin->_list = _next;
        else
	{
	    while (T && T->_next != this) T = T->_next;
            if (T) T->_next = _next;
	}
        if (_wind)
	{
            XDestroyWindow (_disp->_dpy, _wind);
            XFlush (_disp->_dpy);
	}
    }
    while (_list)
    {
        if (_pwin) _list->_wind = 0;
        delete _list; 
    }
}
*/


X_window *X_window::find (Window w)
{
    X_window *T, *W;

    if (_wind == w) return this;
    for (T = _list, W = 0; T && ! (W = T->find (w)); T = T->_next);
    return W;
}


int X_window::x_resize (int xs, int ys) const
{
    return XResizeWindow (_disp->_dpy, _wind, xs, ys);
}


int X_window::x_move (int xp, int yp) const
{
    return XMoveWindow (_disp->_dpy, _wind, xp, yp);
}


int X_window::x_moveresize (int xp, int yp, int xs, int ys) const
{
    return XMoveResizeWindow (_disp->_dpy, _wind, xp, yp, xs, ys);
}


int X_window::x_set_attrib (unsigned long mask, XSetWindowAttributes *attr) const
{
    return XChangeWindowAttributes (_disp->_dpy, _wind, mask, attr);
} 


int X_window::x_set_win_gravity (int gravity) const
{
    XSetWindowAttributes attr;

    attr.win_gravity = gravity;
    return XChangeWindowAttributes (_disp->_dpy, _wind, CWWinGravity, &attr);
}


int X_window::x_set_bit_gravity (int gravity) const
{
    XSetWindowAttributes attr;

    attr.bit_gravity = gravity;
    return XChangeWindowAttributes (_disp->_dpy, _wind, CWBitGravity, &attr);
}


int X_window::x_add_events (unsigned long events)
{
    _ebits |= events;
    return XSelectInput (_disp->_dpy, _wind, _ebits); 
}


int X_window::x_rem_events (unsigned long events)
{
    _ebits &= ~events;
    return XSelectInput (_disp->_dpy, _wind, _ebits); 
}


int X_window::x_set_title (const char *title)
{
    return XStoreName (_disp->_dpy, _wind, title);
}


int X_window::x_set_background (unsigned long color)
{
    return XSetWindowBackground (_disp->_dpy, _wind, color);
}


void X_window::x_apply (X_hints *hints)
{
    if (hints->_sh.flags) XSetWMNormalHints (_disp->_dpy, _wind, &(hints->_sh));
    if (hints->_mh.flags) XSetWMHints (_disp->_dpy, _wind, &(hints->_mh));
    XSetClassHint (_disp->_dpy, _wind, &(hints->_ch));
    hints->_sh.flags = 0;
    hints->_mh.flags = 0;     
}



X_rootwin::X_rootwin (X_display *disp) :
    X_window (disp)
{
    _disp->_xft = XftDrawCreate (_disp->_dpy, _wind, _disp->_dvi, _disp->_dcm);
}


X_rootwin::~X_rootwin (void)
{
//    if (_disp->_xft) XftDrawDestroy (_disp->_xft);
}

/*
void X_rootwin::handle_event (void)
{
    XEvent E;
    while (   XCheckMaskEvent (_disp->_dpy, ~0L, &E)
	   || XCheckTypedEvent (_disp->_dpy, SelectionNotify, &E)
	   || XCheckTypedEvent (_disp->_dpy, ClientMessage, &E))
    {
	handle_event (&E);
    }
}
*/


static Bool check_event (Display *dpy, XEvent *ev, char *arg)
{
    return True;
}


void X_rootwin::handle_event (void)
{
    XEvent E;
    while (XCheckIfEvent (_disp->_dpy, &E, &check_event, 0))
    {
	handle_event (&E);
    }
}


void X_rootwin::handle_event (XEvent *E)
{
    X_window *W;
  
    W = find (((XAnyEvent *) E)->window);
    if (W && W != this) W->handle_event (E);
    XFlush (_disp->_dpy);
}


