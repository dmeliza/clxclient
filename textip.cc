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


#include <X11/keysym.h>
#include "clxclient.h"


#define DX 8


X_textln::X_textln (X_window          *parent, 
                    X_textln_style    *style,
                    int xp,
                    int yp,
                    int xs,
                    int ys,
                    const char *text,
                    int xal) :
    X_window (parent, xp, yp, xs, ys, style->color.normal.bgnd),
    _style (style),
    _text (text)
{
    XGlyphInfo K;

    x_add_events (ExposureMask);
    _len = strlen (text);
    _x0 = 2;
    _y0 = (ys + style->font->ascent - style->font->descent) / 2;
    if (xal >= 0)
    { 
        XftTextExtentsUtf8 (dpy (), _style->font, (const FcChar8 *) _text, _len, &K);
        _x0 = xs - K.width - 2;
        if (xal == 0) _x0 /= 2;
    }
}


X_textln::~X_textln (void)
{
}


void X_textln::handle_event (XEvent *xe)
{
    switch (xe->type)
    {
    case Expose:
	expose ((XExposeEvent *) xe);
	break;  
    }
}


void X_textln::expose (XExposeEvent *e)
{
    if (e->count == 0)
    {
        XClearWindow (dpy (), win ());
        if (_len)
        {
            XftDrawChange (xft (), win ());
            XftDrawStringUtf8 (xft (), _style->color.normal.text, _style->font, _x0, _y0, (const FcChar8 *) _text, _len);
        }
    }
}


XIC           X_textip::_xic = 0;
unsigned char X_textip::_utf8mark [6] = { 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };


X_textip::X_textip (X_window          *parent, 
                    X_callback        *callb,
                    X_textln_style    *style,
                    int xp,
                    int yp,
                    int xs,
                    int ys,
                    int max,
                    X_linked     *back,
                    X_linked     *forw) :
    X_window (parent, xp, yp, xs, ys, style->color.normal.bgnd),
    X_linked (back, forw),
    _callb (callb), 
    _style (style),
    _bg (_style->color.normal.bgnd),
    _fg (_style->color.normal.text),
    _flags (0),
    _xs (xs),
    _ys (ys),
    _txt (0),
    _max (max),
    _ic (0),
    _i1 (0)
{
    x_add_events (ExposureMask);
    _txt = new unsigned char [_max + 1];
    _y0 = (_ys + style->font->ascent - style->font->descent) / 2;
    _x0 = _x1 = _xc = DX;
    if (callb)
    {
	x_add_events (ButtonPressMask | FocusChangeMask);
        if (! _xic) _xic = XCreateIC (xim (), XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
   		                      XNClientWindow, win (), XNFocusWindow, win (), NULL);
    }
}


X_textip::~X_textip (void)
{
    delete [] _txt;
}


void X_textip::set_text (const char *txt)
{
    _i1 = 0;
    if (txt) while ((*((unsigned char *) txt) >= ' ') && (_i1 < _max)) _txt [_i1++] = *txt++;
    if (_flags & RIGHT) { _x0 = _xs - DX; _ic = _i1; }
    else                { _x0 = DX;       _ic = 0;   }
    xorcursor ();
    update (true);
}
   

void X_textip::set_color (unsigned long bg, XftColor *fg)
{
    if ((_fg != fg) || (_bg != bg))
    {
        _fg = fg;
        _bg = bg;
        redraw ();
    }
}


void X_textip::set_align (int k)
{
    if (k > 0)
    {
        _flags &= ~CENTER;
        _flags |=  RIGHT;
        _x0 = _xs - DX;
        _ic = _i1;
    }
    else if (k < 0)
    {
        _flags &= ~(RIGHT | CENTER);
        _x0 = DX;
        _ic = 0;
    }
    else
    {
        _flags |=  CENTER;
        _flags &= ~RIGHT;
    }
    xorcursor ();
    update (true);
}
   

void X_textip::enable (void)
{
    XSetInputFocus (dpy (), win (), RevertToPointerRoot, CurrentTime);
}


void X_textip::select (void)
{
    if (_callb) _callb->handle_callb (X_callback::TEXTIP | BUT, this, 0);
}


void X_textip::handle_event (XEvent *xe)
{
    switch (xe->type)
    {
    case Expose:
	expose ((XExposeEvent *) xe);
	break;  
 
    case ButtonPress:
	bpress ((XButtonEvent *) xe); 
	break;  
 
    case KeyPress:
	keypress ((XKeyEvent *) xe);
	break;

    case FocusIn:
	setfocus ((XFocusChangeEvent *) xe);
	break;

    case FocusOut:
	remfocus ((XFocusChangeEvent *) xe);
	break;

    case SelectionNotify:
	paste ((XSelectionEvent *) xe);
	break;
    }
}


void X_textip::expose (XExposeEvent *e)
{
    if (e->count == 0) redraw ();
}


void X_textip::bpress (XButtonEvent *e)
{
    if (e->button == Button2)
    {
    }
    else
    {
	xorcursor (); 
	_ic = findindex (e->x); 
	_xc = _x0 + textwidth (0, _ic);
        if (_callb) _callb->handle_callb (X_callback::TEXTIP | BUT, this, (XEvent *) e);
	xorcursor ();
    }
}


void X_textip::keypress (XKeyEvent *e)
{
    int  k;
    unsigned char t [8];

    if (XFilterEvent ((XEvent *) e, win ())) return;
    if (   (k = Xutf8LookupString (_xic, e, (char *) t, 8, 0, 0))
        && ((*t >= 192) || ((*t >= 32) && (*t < 127))))
    {
	checkclear ();
        insert (k, t);
	xorcursor ();
        update (true);
	checkcallb ();
    }
    else spkey (e);
}


int X_textip::test_utf8 (int k, const unsigned char *t)
{
    int c, i, n;

    c = *t++;
    if ((c < 0x20) || (c == 0x7F) || (c >= 0xFE)) return 0;  
    if (c < 0x7F) return 1;
    for (n = 5; n;  n--) 
    {
	if (c >= _utf8mark [n])
	{
            if (n >= k) return 0;
	    for (i = 0; i < n; i++) 
	    {
		c = *t++;
                if ((c < 0x80) || (c >= 0xC0)) return 0;
	    }
            return n + 1;
	} 
    }
    return 0;
}


void X_textip::insert (int k, const unsigned char *t)
{
    int i;

    if (_i1 + k <= _max)
    {
	_i1 += k;
	for (i = _i1; i > _ic; i--) _txt [i] = _txt [i - k];
        while (k--) _txt [_ic++] = *t++;
    }
    else XBell (dpy (), 0);    
}


void X_textip::paste (XSelectionEvent *e)
{
    int                   k, n;
    const unsigned char  *p;
    XTextProperty         t;

    if (e->target != None)
    {
	XGetTextProperty (dpy (), win (), &t, e->property);
	n = t.nitems;
	if (n > _max - _i1) XBell (dpy (), 0);
	else if (n)
	{
            checkclear ();
            p = (const unsigned char *)(t.value);
            while (n)
	    {
		if ((k = test_utf8 (n, p)))
		{
		    insert (k, p);
                    n -= k;
                    p += k;
		}
                else 
		{
		    k--;
		    p++;
		}
	    }
	    xorcursor ();
            update (true);
            checkcallb ();
	}     
    }
}


void X_textip::spkey (XKeyEvent *e)
{
    int k = XLookupKeysym (e, 0);

    switch (k)
    {
    case XK_Home:	
    case 'a':
	go_sol ();
	break;

    case XK_End:	
    case 'e':
	go_eol ();
	break;

    case XK_Left:
        if (e->state & (ControlMask | ShiftMask)) cbkey (k, e);
   	else go_lt ();
	break;

    case 'b':
	go_lt ();
	break;

    case XK_Right:
	if (e->state & (ControlMask | ShiftMask)) cbkey (k, e);
   	else go_rt ();
	break;

    case 'f':
	go_rt ();
	break;

    case XK_Delete:
    case 'd':
	del_rt ();
	break;

    case 'k':
	del_eol ();
	break;

    case 'u':
	del_all ();
	break;

    case XK_BackSpace:
	del_lt ();
	break;

    case XK_Tab:
	go_eol ();
        if (e->state & ControlMask) { if (_back) _back->select (); }
        else                        { if (_forw) _forw->select (); }          
        break;

    case XK_Up:
    case XK_Down:
	cbkey (k, e);
	break;

    case XK_Return:
    case XK_KP_Enter:
	go_eol ();
	if (_forw) _forw->select ();
	else cbkey (XK_Return, e);
	break;

    default:
	cbkey (k, e);
    }
}


void X_textip::cbkey (int k, XKeyEvent *e)
{
    _key = k; 
    if (_callb) _callb->handle_callb (X_callback::TEXTIP | KEY, this, (XEvent *) e);
}


void X_textip::go_sol (void)
{
    _flags &= ~CLEAR; 
    if (_i1)
    {
        xorcursor ();
	_ic = 0;
	_xc = _x0;
	update (false);
    }
}


void X_textip::go_eol (void)
{
    _flags &= ~CLEAR; 
    if (_i1)
    {
        xorcursor ();
	_ic = _i1;
	_xc = _x1;
	update (false);
    }
}


void X_textip::go_lt (void)
{
    int i, d;

    _flags &= ~CLEAR; 
    if (_ic > 0)
    {
        for (i = _ic - 1, d = 1; (i > 0) && (_txt [i] >= 0x80) && (_txt [i] < 0xC0); i--, d++);
	xorcursor ();
        _ic -= d;
	_xc = _x0 + textwidth (0, _ic);
	update (false);
    }
}


void X_textip::go_rt (void)
{
    int i, d;

    _flags &= ~CLEAR; 
    if (_ic < _i1)
    {
        for (i = _ic + 1, d = 1; (i < _i1) && (_txt [i] >= 0x80) && (_txt [i] < 0xC0); i++, d++);
	xorcursor ();
        _ic += d;       
	_xc = _x0 + textwidth (0, _ic);   
	update (false);
    }
}


void X_textip::del_lt (void)
{
    int i, d;

    _flags &= ~CLEAR; 
    if (_ic > 0)
    {
        for (i = _ic - 1, d = 1; (i > 0) && (_txt [i] >= 0x80) && (_txt [i] < 0xC0); i--, d++);
        _ic -= d;
        _i1 -= d;
	for (i = _ic; i < _i1; i++) _txt [i] = _txt [i + d];
	xorcursor ();
        update (true);
        checkcallb ();
    }    
}


void X_textip::del_rt (void)
{
    int i, d;

    _flags &= ~CLEAR; 
    if (_ic < _i1)
    {
        for (i = _ic + 1, d = 1; (i < _i1) && (_txt [i] >= 0x80) && (_txt [i] < 0xC0); i++, d++);
        _i1 -= d;        
	for (i = _ic; i < _i1; i++) _txt [i] = _txt [i + d];
	xorcursor ();
        update (true);
        checkcallb ();
    }    
}


void X_textip::del_eol (void)
{
    _flags &= ~CLEAR; 
    if (_ic < _i1)
    {
	_i1 = _ic;
	xorcursor ();
        update (true);
        checkcallb ();
    }
}


void X_textip::del_all (void)
{
    _flags &= ~CLEAR; 
    if (_i1)
    {
	_ic = _i1 = 0;
        _xc = (_flags & RIGHT) ? _xs - DX : DX;
        _x0 = _x1 = _xc; 
	redraw ();
        checkcallb ();
    }
}


void X_textip::update (bool f)
{
    int d;

    _xc = _x0 + textwidth (0, _ic);
    _x1 = _xc + textwidth (_ic, _i1 - _ic);

    if (_flags & CENTER)
    {
	d = (_x0 +  _x1 - _xs) / 2;
        if (d > _xc - DX) d = _xc - DX;
        if (d < _xc - _xs + DX) d = _xc - _xs + DX;
    }
    else if (_flags & RIGHT)
    {
        d = _x1 - _xs + DX;
        if (d > _xc - DX) d = _xc - DX;
    }
    else
    {
	d = _x0 - DX;
        if (d < _xc - _xs + DX) d = _xc - _xs + DX;
    }
    _xc -= d;
    _x0 -= d;
    _x1 -= d;
    if (d | f) redraw ();
    else xorcursor ();
}


void X_textip::redraw (void)
{
    GC gc = disp ()->dgc ();
    XftDraw         *D = xft ();
    XftColor        *C;

    XSetWindowBackground (dpy (), win (), (_flags & FOCUS) ? _style->color.focus.bgnd : _bg);
    XClearWindow (dpy (), win ());

    if (_i1)
    {
        XftDrawChange (D, win ());
	C = (_flags & FOCUS) ? _style->color.focus.text : _fg;
        XftDrawStringUtf8 (D, C, _style->font, _x0, _y0, (const FcChar8 *) _txt, _i1);
    }
    if (_callb)
    {
	XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
        XSetFunction (dpy (), gc, GXcopy);
	XSetForeground (dpy (), gc, _style->color.shadow.dark);
	XDrawLine (dpy (), win (), gc,  0, 0, 0, _ys - 1);
	XDrawLine (dpy (), win (), gc,  0, 0, _xs - 1, 0);
	XSetForeground (dpy (), gc, _style->color.shadow.lite);
	XDrawLine (dpy (), win (), gc, _xs - 1, 1, _xs - 1, _ys);
	XDrawLine (dpy (), win (), gc, 1, _ys - 1, _xs, _ys - 1);
	XSetForeground (dpy (), gc, _style->color.shadow.bgnd);
        XDrawPoint (dpy (), win (), gc, 0, _ys - 1);
        XDrawPoint (dpy (), win (), gc, _xs - 1, 0);
    }
    xorcursor ();
}


void X_textip::setfocus (XFocusChangeEvent *e)
{
    if (e->detail != NotifyPointer && ! (_flags & FOCUS))
    {  
	_flags ^= FOCUS;
	x_add_events (KeyPressMask);
        XSetICValues (_xic, XNFocusWindow, win (), NULL);
	redraw ();
    }
}


void X_textip::remfocus (XFocusChangeEvent *e)
{
    if (_flags & FOCUS)
    {  
	_flags ^= FOCUS;
	x_rem_events (KeyPressMask);
	redraw ();
    }
}


void X_textip::xorcursor (void)
{
    int y0, y1;

    if (_flags & FOCUS)
    {
	GC gc = disp ()->dgc ();
	XSetForeground (dpy (), gc, _style->color.focus.bgnd ^ _style->color.focus.line);
	XSetFunction (dpy (), gc, GXxor);
	XSetLineAttributes (dpy (), gc, 1, LineSolid, CapButt, JoinBevel);
        y0 = _y0 - _style->font->ascent;
        y1 = _y0 + _style->font->descent;
        if (y0 < 1) y0 = 1;
        if (y1 > _ys - 1) y1 = _ys - 1;
	XDrawLine (dpy (), win (), gc, _xc, y0, _xc, y1);
    }
} 


int X_textip::findindex (int x)
{
    int i, i0, i1;

    if (x < _x0) return 0;
    i0 = 0;
    i1 = _i1;
    while (i1 > i0 + 1)  
    {
	i = (i0 + i1) / 2;
	if (_x0 + textwidth (0, i) > x) i1 = i;
	else                            i0 = i;
    }
    return i1;   
}    


int X_textip::textwidth (int i, int n)
{
    XGlyphInfo K;

    if (n == 0) return 0;
     XftTextExtentsUtf8 (dpy (), _style->font, (const FcChar8 *)(_txt + i), n, &K);
     return K.xOff;
}


void X_textip::checkclear (void)
{
    if (_flags & CLEAR) del_all ();
}


void X_textip::checkcallb (void)
{
    if (_flags & CALLB)
    {
        _callb->handle_callb (X_callback::TEXTIP | MODIF, this, 0);
	_flags ^= CALLB;
    }
}
