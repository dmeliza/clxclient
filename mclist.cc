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


#define X0  8
#define DX  28


static char **cmpptr;

extern "C" int compare (const void *a, const void *b)
{
    return strcoll (cmpptr [*(int *)a], cmpptr [*(int *)b]); 
}


X_mclist::X_mclist (X_window  *parent, X_callback *callb, X_mclist_style *style,
                    int xp, int yp, int xs, int ys,
                    int max_item, int max_char) :
    X_window (parent, xp, yp, xs, ys, style->bg), 
    _style (style),
    _callb (callb),
    _max_item (max_item),
    _max_char (max_char),
    _xs (xs),
    _ys (ys),
    _nrow (0),
    _nclm (0)
{
    _buff = new char [max_char];    
    _ptr = new char* [max_item];    
    _len = new short [max_item];    
    _ext = new short [max_item];    
    _col = new short [max_item];    
    _ind = new int [max_item];    
    _dxc = new int [max_item / 4];
    reset ();
    x_add_events (ExposureMask | ButtonPressMask);
}


X_mclist::~X_mclist (void)
{
    delete []_buff; 
    delete []_ptr; 
    delete []_len; 
    delete []_ext;
    delete []_col; 
    delete []_ind;
    delete []_dxc; 
}


void X_mclist::resize (int xs, int ys)
{
    if ((xs != _xs) || (ys != _ys))
    {
	_xs = xs;
        _ys = ys;
        x_unmap ();
        x_resize (_xs, _ys);
        x_map ();
        show ();
    }   
}


void X_mclist::forw (void)
{
}


void X_mclist::back (void)
{
}


void X_mclist::handle_event (XEvent *E)
{
    switch (E->type)
    {
    case Expose:
    case GraphicsExpose:
	expose ((XExposeEvent *) E);
	break;  

    case ButtonPress:
	bpress ((XButtonEvent *) E);
	break;  
    }
}


void X_mclist::expose (XExposeEvent *E)
{
    if (_nclm) update (E->x, E->y, E->width, E->height);
}


void X_mclist::bpress (XButtonEvent *E)
{
    int d, x, y, r, c;

    if (! _nclm) return;
    x = E->x - X0 + _offs;
    y = E->y;
    d = _style->dy;
    r = y / d;
    y -= r * d;
    if ((y < 2) || (y > d - 2)) return;
    for (c = 0; c < _nclm; c++)
    {
        d = _dxc [c];
	if ((x > 0) && (x < d))
	{
	    _sel = _ind [c * _nrow + r]; 
            if (_sel < _n_item) _callb->handle_callb (X_callback::MCLIST | SEL, this, (XEvent *) E);    
            return;
	}
        x -= d + DX;
    }
}


int X_mclist::item (const char *txt, int col, int len)
{
    int         i;
    XGlyphInfo  G;

    if (! len) len = strlen (txt);
    if (! len) return 0;
    i = _n_item; 
    if (i == _max_item) return 1; 
    if (_n_char + len + 1 > _max_char) return 2;
    XftTextExtentsUtf8 (dpy (), _style->font, (const FcChar8 *) txt, len, &G);
    _ptr [i] = _buff + _n_char;
    _len [i] = len;
    _ext [i] = G.width;
    _col [i] = col;
    _ind [i] = i;
    strcpy (_buff + _n_char, txt);
    _n_char += len + 1;
    _n_item = ++i;
    return 0;
}


void X_mclist::sort (void)
{
    cmpptr = _ptr;
    qsort (_ind, _n_item, sizeof (int), compare);
}


void X_mclist::show (void)
{
    int  i, j, r, c, d;

    _nrow = _ys / _style->dy;
    _nclm = (_n_item + _nrow - 1) / _nrow;
    _span = X0;
    r = c = d = 0;
    for (i = 0; i < _n_item; i++)
    {
        j = _ind [i];
        if (d < _ext [j]) d = _ext [j];
        if (++r == _nrow)
	{
            if (c) _span += DX;
            _span += d;            
	    _dxc [c++] = d;
            d = r = 0;
	} 
    }
    if (d)
    {
        if (c) _span += DX;
        _span += d;            
        _dxc [c++] = d;
    }
    _span += X0;
    _offs = 0;
    _sel = -1;
    XClearWindow (dpy (), win ());
    update (0, 0, _xs, _ys);
}


void X_mclist::move (int offs)
{
    int       d;
    XGCValues G;

    d = offs - _offs;
    G.graphics_exposures = True;
    G.function = GXcopy;
    XChangeGC (dpy (), dgc (), GCGraphicsExposures | GCFunction, &G);
    XCopyArea (dpy (), win (), win (), dgc (), d, 0, _xs, _ys, 0, 0);
    G.graphics_exposures = False;
    XChangeGC (dpy (), dgc (), GCGraphicsExposures, &G);
    _offs = offs;
}


void X_mclist::update (int xx, int yy, int ww, int hh)
{
    int            i, j, c, r;
    int            x, dx, y, ya, yd, yb, dy;
    XftDraw       *D = xft ();
    XftColor      *C;
    XftFont       *F = _style->font;

    ya = _style->font->ascent;
    yd = _style->font->descent;
    dy = _style->dy;
    yb = (dy + ya - yd) / 2;
    XSetForeground (dpy (), dgc (), _style->bg);
    XSetFunction (dpy (), dgc (), GXcopy);
    XftDrawChange (D, win ());

    x = X0 - _offs;
    for (c = 0; c < _nclm; c++)
    {
	dx = _dxc [c];
        if (x >= xx + ww) break;
        if (x + dx > xx)
	{
            y = 0;
            for (r = 0; r < _nrow; r++)
	    {
   	        i = c * _nrow + r;
                if (i >= _n_item) break;
                if (y + yb - ya >= yy + hh) break;           
                if (y + yb + yd > yy)
		{
		    j = _ind [i];
                    if (x + _ext [j] > xx)
	            {
			XFillRectangle (dpy (), win (), dgc (), x, y, dx, dy); 
  		        C = _style->fg [_col [j] & 3];
			XftDrawStringUtf8 (D, C, F, x, y + yb, (const FcChar8 *) _ptr [j], _len [j]);
		    }
		}
                y += dy;
	    }
	}
        x += dx + DX;
    }
}

