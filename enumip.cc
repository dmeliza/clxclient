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


#include <clxclient.h>
#include <X11/keysym.h>


#define DX 8


XIC           X_enumip::_xic = 0;
unsigned char X_enumip::_utf8mark [6] = { 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };


X_enumip::X_enumip (X_window        *parent, 
                    X_callback      *callb,
                    X_textln_style  *style,
                    int xp,
                    int yp,
                    int xs,
                    int ys,
                    X_enip_item  *list,
                    X_linked     *back,
                    X_linked     *forw) :
    X_window (parent, xp, yp, xs, ys, style->color.normal.bgnd),
    X_linked (back, forw),
    _callb (callb), 
    _style (style),
    _list (list),
    _bg (_style->color.normal.bgnd),
    _fg (_style->color.normal.text),
    _txt (0),
    _ind (0),
    _len (0),
    _foc (0),
    _xs (xs),
    _ys (ys),
    _x0 (xs - DX)
{
    int i, l, m;

    x_add_events (ExposureMask | ButtonPressMask | FocusChangeMask);
    _y0 = (_ys + style->font->ascent - style->font->descent) / 2;
    if (callb)
    {
	x_add_events (ButtonPressMask | FocusChangeMask);
        if (! _xic) _xic = XCreateIC (xim (), XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
   		                      XNClientWindow, win (), XNFocusWindow, win (), NULL);
    }
    if (_list)
    {
	for (i = m = 0; _list [i]._text; i++)
	{
	    l = strlen (list [i]._text);
	    if (l > m) m = l;
	}
	_max = i;
	_txt = new char [m];
    }
    else _max = 0;
}


X_enumip::~X_enumip (void)
{
    delete [] _txt;
}


void X_enumip::set_ind (int ind)
{
    if ((ind < 0) || (ind >= _max))
    {
	_ind = -1;
	_len = 0;
    }
    else
    {
	strcpy (_txt, _list [_ind = ind]._text);
	_len = strlen (_txt);
    }
    _x0 = _xs - DX;
    if (_len) _x0 -= textwidth (0, _len);
    redraw ();
}
   

void X_enumip::select (void)
{
    if (_callb) _callb->handle_callb (X_callback::ENUMIP | BUT, this, 0);
}


void X_enumip::handle_event (XEvent *xe)
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
    }
}


void X_enumip::expose (XExposeEvent *e)
{
    redraw ();
}


void X_enumip::bpress (XButtonEvent *e)
{
    if (e->button == Button2)
    {
	//    if (_foc) XConvertSelection (dpy (), XA_PRIMARY, XA_STRING, XInternAtom (dpy (),
	//                                 "XTIPSEL", False), win (), e->time);
    }
    else
    {
	select ();
    }
}


void X_enumip::keypress (XKeyEvent *e)
{
    int  k;
    unsigned char t [8];

    if (XFilterEvent ((XEvent *) e, win ())) return;
    if (   (k = Xutf8LookupString (_xic, e, (char *) t, 8, 0, 0))
        && ((*t >= 192) || ((*t >= 32) && (*t < 127))))
    {
    }
    else spkey (e);
}


void X_enumip::spkey (XKeyEvent *e)
{
    int k = XLookupKeysym (e, 0);

    switch (k)
    {
    case XK_Left:
    case 'b':
	if (--_ind < 0) _ind = _max - 1;
	set_ind (_ind);
	break;

    case XK_Right:
    case 'f':
	if (++_ind == _max) _ind = 0;
	set_ind (_ind);
	break;

    case XK_Tab:
        if (e->state & ControlMask) { if (_back) _back->select (); }
        else                        { if (_forw) _forw->select (); }          
        break;

    case XK_Up:
    case XK_Down:
	cbkey (k);
	break;

    case XK_Return:
    case XK_KP_Enter:
	if (_forw) _forw->select ();
	else cbkey (XK_Return);
	break;

    default:
	cbkey (k);
    }
}


void X_enumip::cbkey (int k)
{
    _key = k; 
    if (_callb) _callb->handle_callb (X_callback::ENUMIP | KEY, this, 0);
}


void X_enumip::redraw (void)
{
    GC       gc = dgc ();
    XftDraw  *D = xft ();
    XftColor *C;

    XClearWindow (dpy (), win ());
    if (_len)
    {
        XftDrawChange (D, win ());
	C = _foc ? _style->color.focus.text : _fg;
        XftDrawStringUtf8 (D, C, _style->font, _x0, _y0, (const FcChar8 *) _txt, _len);
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
}


void X_enumip::setfocus (XFocusChangeEvent *e)
{
    if (e->detail != NotifyPointer && ! _foc)
    {  
	_foc = 1;
	x_add_events (KeyPressMask);
	XSetWindowBackground (dpy (), win (),_style->color.focus.bgnd);
        XSetICValues (_xic, XNFocusWindow, win (), NULL);
	redraw ();
    }
}


void X_enumip::remfocus (XFocusChangeEvent *e)
{
    if (_foc)
    {  
	_foc = 0;
	x_rem_events (KeyPressMask);
	XSetWindowBackground (dpy (), win (), _bg);
	redraw ();
    }
}


int X_enumip::textwidth (int i, int n)
{
    XGlyphInfo K;

    if (n == 0) return 0;
     XftTextExtentsUtf8 (dpy (), _style->font, (const FcChar8 *)(_txt + i), n, &K);
     return K.xOff;
}


 
