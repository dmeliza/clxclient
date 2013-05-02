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


X_button::X_button (X_window       *parent,
                    X_callback     *callb,
                    X_button_style *style,
                    int xp, int yp,
                    int  cbid) :
    X_window (parent, xp, yp, style->size.x, style->size.y, style->color.shadow.bgnd),
    _style (style),
    _callb (callb),
    _xs (style->size.x),
    _ys (style->size.y),
    _cbid (cbid),
    _down (0),
    _stat (0)
{
    x_add_events (ExposureMask);
    if (_callb) x_add_events (ButtonPressMask | ButtonReleaseMask | Button1MotionMask | LeaveWindowMask);
}


X_button::~X_button (void)
{
}


void X_button::handle_event (XEvent *E)
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
        _callb->handle_callb (X_callback::BUTTON | MOVE, this, E);
	break;

    case LeaveNotify:
	release ();
	break;
    }
}


void X_button::bpress (XButtonEvent *E)
{
    _down = 1;
    redraw ();
    _callb->handle_callb (X_callback::BUTTON | PRESS, this, (XEvent *) E);
}


void X_button::brelease (XButtonEvent *E)
{
    _down = 0;
    redraw ();
    _callb->handle_callb (X_callback::BUTTON | RELSE, this, (XEvent *) E);
}


void X_button::expose (XExposeEvent *E)
{
    if (E->count == 0) redraw ();
}


void X_button::release (void)
{
    XUngrabPointer (dpy (), CurrentTime);
    _down = 0;
    redraw ();
}


void X_button::set_stat (int stat)
{
    if (_stat != stat)
    {
	_stat = stat;
        redraw ();
    }
}


void X_button::redraw (void)
{
    GC gc = disp ()->dgc ();

    XSetFunction (dpy (), gc, GXcopy);
    XSetLineAttributes (dpy (), gc, 0, LineSolid, CapButt, JoinBevel);
    switch (_style->type & 3)
    {
    case X_button_style::RAISED:
	XSetForeground (dpy (), gc, _down ? _style->color.shadow.dark : _style->color.shadow.lite);
	XDrawLine (dpy (), win (), gc,  0, 0, 0, _ys - 2);
	XDrawLine (dpy (), win (), gc,  0, 0, _xs - 2, 0);
	XSetForeground (dpy (), gc, _down ? _style->color.shadow.lite : _style->color.shadow.dark);
	XDrawLine (dpy (), win (), gc, _xs - 1, 1, _xs - 1, _ys);
	XDrawLine (dpy (), win (), gc, 1, _ys - 1, _xs, _ys - 1);
        XSetForeground (dpy (), gc, _style->color.shadow.bgnd);
        XDrawPoint (dpy (), win (), gc, 0, _ys - 1);
        XDrawPoint (dpy (), win (), gc, _xs - 1, 0);
	break;
    }
}



X_tbutton::X_tbutton (X_window       *parent,
                      X_callback     *callb,
                      X_button_style *style,
                      int             xp,
                      int             yp,
                      const char     *text1,
                      const char     *text2,
                      int             cbid) :
    X_button (parent, callb, style, xp, yp, cbid),
    _bg (0),
    _fg (0), 
    _len1 (0),
    _len2 (0)
{
    set_text (text1, text2);
}


X_tbutton::~X_tbutton (void)
{
}


void X_tbutton::set_text (const char *text1, const char *text2)
{
    _len1 = 0;
    _len2 = 0;
    if (text1) while (((*(unsigned char *) text1) >= ' ') && (_len1 < MAXLEN - 1)) _text1 [_len1++] = *text1++;
    if (text2) while (((*(unsigned char *) text2) >= ' ') && (_len2 < MAXLEN - 1)) _text2 [_len2++] = *text2++;
    _text1 [_len1] = 0;
    _text2 [_len2] = 0;
}


void X_tbutton::redraw (void)
{
    int            a, d, u, x, y;
    GC             G = dgc ();
    XftDraw       *D = xft ();
    XftColor      *C;
    XftFont       *F = _style->font;
    XGlyphInfo     K;
    unsigned long  b;

    XftDrawChange (D, win ());
    XSetFunction (dpy (), G, GXcopy);
    b = (_stat > 3) ? _bg : _style->color.bg [_stat];
    XSetForeground (dpy (), G, b);
    XFillRectangle (dpy (), win (), G, 0, 0, _xs - 1, _ys - 1);

    a = F->ascent;
    d = F->descent;
    u = a + d - _ys / 2;
    if (u < 0) u = 0;

    C = (_stat > 3) ? _fg : _style->color.fg [_stat];
    if (_len1)
    {
	y = (_len2) ? _ys / 2 - d + u : (_ys + a - d) / 2;
	if (_style->type & X_button_style::ALEFT) x = 6;
        else
	{  
            XftTextExtentsUtf8 (dpy (), F, (const FcChar8 *) _text1, _len1, &K);
	    if (_style->type & X_button_style::ARIGHT) x = _xs - K.width - 6;
	    else x = (_xs - K.width) / 2;
	}
        XftDrawStringUtf8 (D, C, F, x, y, (const FcChar8 *) _text1, _len1);
    }
    if (_len2)
    {
	y = (_len1) ? _ys / 2 + a - u : (_ys + a - d) / 2;
	if (_style->type & X_button_style::ALEFT) x = 6;
	else
	{
            XftTextExtentsUtf8 (dpy (), F, (const FcChar8 *) _text2, _len2, &K);
            if (_style->type & X_button_style::ARIGHT) x = _xs - K.width - 6;
	    else x = (_xs - K.width) / 2;
	}
        XftDrawStringUtf8 (D, C, F, x, y, (const FcChar8 *) _text2, _len2);
    }
    if ((_style->type & 3) == X_button_style::BORDER)
    {
        XSetLineAttributes (dpy (), G, 1, LineSolid, CapButt, JoinBevel);
	XSetForeground (dpy (), G, C->pixel);
	XDrawRectangle (dpy (), win (), G, 0, 0, _xs - 1, _ys - 1);
    }
    else X_button::redraw ();
}



X_ibutton::X_ibutton (X_window       *parent,
                      X_callback     *callb,
                      X_button_style *style,
                      int             xp,
                      int             yp,
                      XImage         *image, 
                      int             cbid) :
    X_button (parent, callb, style, xp, yp, cbid),
    _bg (0),
    _fg (0),
    _image (image)
{
}


X_ibutton::~X_ibutton (void)
{
}


void X_ibutton::redraw (void)
{
    int           x, y;
    unsigned long b, f;

    b = (_stat > 3) ? _bg : _style->color.bg [_stat];
    f = (_stat > 3) ? _fg : _style->color.fg [_stat]->pixel;
    XSetState (dpy (), dgc (), f, b, GXcopy, ~0);
    XSetWindowBackground (dpy (), win (), b);
    XClearWindow (dpy (), win ());
    if (_image)
    {
	x = (_xs - _image->width) / 2;
        y = (_ys - _image->height) / 2; 
        XPutImage (dpy (), win (), dgc (), _image, 0, 0, x, y, _xs, _ys);
    }
    if ((_style->type & 3) == X_button_style::BORDER)
    {
        XSetLineAttributes (dpy (), dgc (), 1, LineSolid, CapButt, JoinBevel);
	XSetForeground (dpy (), dgc (), f);
	XDrawRectangle (dpy (), win (), dgc (), 0, 0, _xs - 1, _ys - 1);
    }
    else X_button::redraw ();
}



X_pbutton::X_pbutton (X_window       *parent,
                      X_callback     *callb,
                      X_button_style *style,
                      int xp, int yp,
                      Pixmap map0,
                      Pixmap map1,
                      int  cbid) :
    X_button (parent, callb, style, xp, yp, cbid),
    _map0 (map0),
    _map1 (map1)
{
    if (_map0) XSetWindowBackgroundPixmap (dpy (), win (), _map0);
}


X_pbutton::~X_pbutton (void)
{
}


void X_pbutton::set_stat (bool stat)
{
    if (_stat != stat)
    {
        if (_map1) XSetWindowBackgroundPixmap (dpy (), win (), stat ? _map1 : _map0);
        _stat = stat;
        XClearWindow (dpy (), win ());
        X_button::redraw ();
    }
}

