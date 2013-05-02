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


#ifndef __CLXCLIENT_H
#define __CLXCLIENT_H


#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xft/Xft.h>
#include <clthreads.h>


// -------------------------------------------------------------------------------------------

const char *clxclient_version (void);

// -------------------------------------------------------------------------------------------


class X_display
{
public:

    X_display (const char *dispname);
    ~X_display (void);

    unsigned long blackpixel (void) const { return BlackPixel (_dpy, _dsn); }
    unsigned long whitepixel (void) const { return WhitePixel (_dpy, _dsn); }

    Display *dpy (void) const { return _dpy; }
    int      dsn (void) const { return _dsn; }
    Colormap dcm (void) const { return _dcm; }
    GC       dgc (void) const { return _dgc; }
    Visual  *dvi (void) const { return _dvi; }
    XftDraw *xft (void) const { return _xft; }
    XIM      xim (void) const { return _xim; }

    int depth (void) const { return DefaultDepth (_dpy, _dsn); }
    int xsize (void) const { return WidthOfScreen  (ScreenOfDisplay (_dpy, _dsn)); }
    int ysize (void) const { return HeightOfScreen (ScreenOfDisplay (_dpy, _dsn)); }
    int gray  (void) const { return 0; }

    unsigned long alloc_color (const char *name, unsigned long d);
    unsigned long alloc_color (float r, float g, float b);
    XftColor *alloc_xftcolor (const char *name, XftColor *D);
    XftColor *alloc_xftcolor (float r, float g, float b, float a);
    void free_xftcolor (XftColor *C);

    XFontStruct *alloc_font  (const char *name);
    XftFont *alloc_xftfont (const char *name);
    void free_xftfont (XftFont *F);

    enum { IMG_LT, IMG_RT, IMG_UP, IMG_DN, IMG_PL, IMG_MI, IMG_SQ, IMG_PT, N_IMG1515 };

    void nofocus (void) { XSetInputFocus (_dpy, PointerRoot, PointerRoot, CurrentTime); }

    XImage *image1515 (unsigned int i);

private:

    friend class X_rootwin;
    friend class X_window;
    friend class X_handler;

    Display     *_dpy;   
    int          _dsn;
    Colormap     _dcm;
    GC           _dgc;
    Visual      *_dvi;
    XftDraw     *_xft;
    XIM          _xim;

    static char  _imgdef1515 [N_IMG1515 * 30];
    XImage      *_imgptr1515 [N_IMG1515];
};


// -------------------------------------------------------------------------------------------


class X_hints
{
public:

    X_hints (void) { _sh.flags = 0; _mh.flags = 0; _ch.res_name = 0; _ch.res_class = 0; }

    void position (int x, int y);
    void size (int x, int y);
    void minsize  (int x, int y);
    void maxsize  (int x, int y);
    void sizeinc (int x, int y);
    void input (int input);
    void state (int state);
    void group (Window group);
    void rname (const char *rname) { _ch.res_name  = (char *) rname; }
    void rclas (const char *rclas) { _ch.res_class = (char *) rclas; }

    friend class X_window;

private:

    XSizeHints  _sh;
    XWMHints    _mh;
    XClassHint  _ch;

};


// -------------------------------------------------------------------------------------------


class X_window
{
public:

    X_window (X_window *pwin, int xpos, int ypos, int xsize, int ysize,
	      unsigned long bgcol, unsigned long bdcol = 0, int bdpix = 0);

    virtual ~X_window (void);    

    X_display*  disp (void) const { return _disp; }

    Display   *dpy (void) const { return _disp->_dpy; } 
    int        dsn (void) const { return _disp->_dsn; }
    Colormap   dcm (void) const { return _disp->_dcm; }
    GC         dgc (void) const { return _disp->_dgc; }
    Visual    *dvi (void) const { return _disp->_dvi; }
    XftDraw   *xft (void) const { return _disp->_xft; }
    XIM        xim (void) const { return _disp->_xim; }
    Window     win (void) const { return _wind; }

    virtual void handle_event (XEvent *E) {}

    int  x_map (void) const { return XMapWindow   (_disp->_dpy, _wind); }
    int  x_mapraised (void) const { return XMapRaised (_disp->_dpy, _wind); }
    int  x_unmap (void) const { return XUnmapWindow (_disp->_dpy, _wind); }
    int  x_clear (void) const { return XClearWindow (_disp->_dpy, _wind); }
    int  x_raise (void) const { return XRaiseWindow (_disp->_dpy, _wind); }
    int  x_move (int xp, int yp) const;
    int  x_resize (int xs, int ys) const;
    int  x_moveresize (int xp, int yp, int xs, int ys) const;
    int  x_set_attrib (unsigned long mask, XSetWindowAttributes *attr) const;
    int  x_set_win_gravity (int gravity) const;
    int  x_set_bit_gravity (int gravity) const;
    int  x_add_events (unsigned long events);
    int  x_rem_events (unsigned long events);
    int  x_set_title (const char *title);
    int  x_set_background (unsigned long color);
    void x_apply (X_hints *hints);

    X_window *pwin (void) { return _pwin; }

    friend class X_rootwin;

private:
  
    X_window (X_display *disp);
    X_window (const X_window&);
    X_window& operator=(const X_window&);
    X_window      *find (Window w);

    unsigned long _ebits;
    X_display     *_disp;
    X_window      *_pwin;
    X_window      *_next;
    X_window      *_list;
    Window         _wind;
};


// -------------------------------------------------------------------------------------------


class X_subwin : public X_window
{
public:

    X_subwin (X_window *pwin, int xpos, int ypos, int xsize, int ysize,
	      unsigned long bgcol, unsigned long bdcol = 0, int bdpix = 0) :
        X_window (pwin, xpos, ypos, xsize, ysize, bgcol, bdcol, bdpix) 
        {}  

    virtual void handle_event (XEvent *E) { pwin ()->handle_event (E); }
};


// -------------------------------------------------------------------------------------------


class X_rootwin : public X_window
{
public:

    X_rootwin (X_display *disp);
    virtual ~X_rootwin (void);    

    void handle_event (void);
    void handle_event (XEvent *E);
    bool check_done (void) const { return _list == 0; }

private:
};


// -------------------------------------------------------------------------------------------


class X_resman
{
public:

    enum { NDEFOPT = 13 };

    X_resman (void);
    ~X_resman (void);
    X_resman (const X_resman&);
    X_resman& operator=(const X_resman&);

    void init (int *argc, char *argv [], char *name, XrmOptionDescRec *opt, int nopt);

    const char *get (const char *res, const char *def) const;
    int   getb (const char *res, int def) const;
    void  geometry (const char *res, int xd, int yd, int bd,
		    int &xp, int &yp, int &xs, int &ys);

    const char *rname (void) { return _rname; }
    const char *rclas (void) { return _rclas; }

private:
  
    char         _rname [64];
    char         _rclas [64];
    char        *_home;  
    XrmDatabase  _xrmdata;

    static XrmOptionDescRec defopt [X_resman::NDEFOPT];
};



// -------------------------------------------------------------------------------------------


extern "C" int x_handler_error (Display *dpy, XErrorEvent *err);


class X_handler : public H_thread
{
public:

    X_handler (X_display *disp, Edest *dest, int ipid);
    ~X_handler (void);

    void thr_main (void);
    void next_event (void) { put_event (ITC_ip1q::N_MQ, 1); }

private:

    int  _conn;
};


// -------------------------------------------------------------------------------------------


class X_callback 
{
public:

    virtual ~X_callback (void) {}

    enum
    {
	BUTTON    = 0x010000,
	TEXTIP    = 0x010100,
	ENUMIP    = 0x010200,
	ROTARY    = 0x010300,
	SLIDER    = 0x010400,
        SCROLL    = 0x010500,
	MENU      = 0x010600,
	MCLIST    = 0x010700
    };

    virtual void handle_callb (int /*type*/, X_window */*W*/, XEvent */*E*/) = 0;
    static int cb_class (int k) { return k & 0xFFFFFF00; }
    static int cb_event (int k) { return k & 0x000000FF; }
};


// -------------------------------------------------------------------------------------------


class X_draw
{
public:

    X_draw (Display *dpy, Drawable drw, GC gct, XftDraw *xft);
    ~X_draw (void) {}

    void setfont (XFontStruct *font) { _xft_font = 0; _x11_font = font; XSetFont (_dpy, _gct, font->fid); }
    void setfont (XftFont     *font) { _x11_font = 0; _xft_font = font; }
    void setcolor (unsigned long c) { _xft_color = 0;  XSetForeground (_dpy, _gct, c); }
    void setcolor (XftColor     *c) { _xft_color = c;  XSetForeground (_dpy, _gct, c->pixel); }
    void setfunc (int f) { XSetFunction (_dpy, _gct, f); }
    void setline (int d) { XSetLineAttributes (_dpy, _gct, d, LineSolid, CapButt, JoinBevel); } 
    void setline (int d, int m) { XSetLineAttributes (_dpy, _gct, d, m, CapButt, JoinBevel); } 
    void setdash (char *v, int n, int i = 0) { XSetDashes (_dpy, _gct, i, v, n); }
    void move (int x, int y) { _xx = x; _yy = y; }
    void rmove (int x, int y) { _xx += x; _yy += y; }
    void draw (int x, int y) { XDrawLine (_dpy, _drw, _gct, _xx, _yy, x, y); _xx = x; _yy = y; }
    void rdraw (int x, int y) { XDrawLine (_dpy, _drw, _gct, _xx, _yy, _xx + x, _yy + y); _xx += x; _yy += y; }
    void drawline (int x0, int y0, int x1, int y1) { XDrawLine (_dpy, _drw, _gct, x0, y0, x1, y1); }
    void drawrect (int x0, int y0, int x1, int y1) { XDrawRectangle (_dpy, _drw, _gct, x0, y0, x1 - x0, y1 - y0); }
    void fillrect (int x0, int y0, int x1, int y1) { XFillRectangle (_dpy, _drw, _gct, x0, y0, x1 - x0, y1 - y0); }
    void drawrect (int dx, int dy) { XDrawRectangle (_dpy, _drw, _gct, _xx, _yy, dx, dy); }
    void fillrect (int dx, int dy) { XFillRectangle (_dpy, _drw, _gct, _xx, _yy, dx, dy); }
    void drawarc (int dx, int dy, int a1, int a2)  { XDrawArc (_dpy, _drw, _gct, _xx, _yy, dx, dy, a1, a2); }
    void fillarc (int dx, int dy, int a1, int a2)  { XFillArc (_dpy, _drw, _gct, _xx, _yy, dx, dy, a1, a2); }
    void movepix (int dx, int dy, int xs, int ys);
    void setclip (int x0, int y0, int x1, int y1);
    void noclip (void) { XSetClipMask (_dpy, _gct, None); }
    void drawpoints (int n, XPoint *P);
    void drawlines (int n, XPoint *P);
    void drawsegments (int n, XSegment *S);
    void putimage (XImage *I) { XPutImage (_dpy, _drw, _gct, I, 0, 0, _xx, _yy, I->width, I->height); }
    void clearwin (void) { XClearWindow (_dpy, _drw); }

    int  textwidth (const char *str);
    int  textascent  (void) { return _xft_font ? _xft_font->ascent  : (_x11_font ? _x11_font->ascent  : 0); }
    int  textdescent (void) { return _xft_font ? _xft_font->descent : (_x11_font ? _x11_font->descent : 0); }
    void drawstring (const char *str, int xal);

private:
  
    Display      *_dpy;
    Drawable      _drw;
    GC            _gct;
    XftDraw      *_xft_draw;
    XftColor     *_xft_color;
    XftFont      *_xft_font;
    XFontStruct  *_x11_font;
    int           _xx;
    int           _yy;
};


// -------------------------------------------------------------------------------------------


class X_linked 
{
private:

    X_linked (X_linked *back, X_linked *forw) :
	_back (back),
	_forw (forw)
	{
	    if (back) back->_forw = this;
	    if (forw) forw->_back = this;
	}

    virtual ~X_linked (void)
	{
	    if (_back) _back->_forw = _forw;
	    if (_forw) _forw->_back = _back;
	}

    virtual void select (void) = 0;

    friend class X_textip;
    friend class X_enumip;

    X_linked *_back;
    X_linked *_forw;
};


// -------------------------------------------------------------------------------------------


class X_button_style
{
public:

    enum { PLAIN = 0, BORDER = 1, RAISED = 2, ALEFT = 4, ARIGHT = 8 };

    XftFont *font;
    struct 
    {
	unsigned long bg [4];
	XftColor     *fg [4];
	struct 
	{ 
	    unsigned long bgnd;
	    unsigned long lite;
	    unsigned long dark;
	} shadow;
    } color;
    struct 
    {
	int x;
	int y;
    } size;
    int type;
};


class X_button : public X_window
{
public:

    enum { MOVE, PRESS, RELSE };

    int  cbid (void) const { return _cbid; }
    int  stat (void) const { return _stat; } 
    void set_cbid (int cbid) { _cbid = cbid; }
    void release (void);
    virtual void set_stat (int stat);
    virtual void redraw (void);

protected:

    X_button (X_window       *parent,
	      X_callback     *callb,
	      X_button_style *style,
	      int xp,
	      int yp,
	      int cbid);

    ~X_button (void);

    void handle_event (XEvent *E);
    void expose (XExposeEvent *E);
    void bpress (XButtonEvent *E);
    void brelease (XButtonEvent *E);

    const X_button_style *_style;
    X_callback     *_callb;
    int             _xs;
    int             _ys;
    int             _cbid;  
    int             _down;
    int             _stat;
};


class X_tbutton : public X_button
{
public:

    enum { MAXLEN = 24 };

    X_tbutton (X_window       *parent,
	       X_callback     *callb,
	       X_button_style *style,
	       int             xp,
	       int             yp,
	       const char     *text1 = 0,
	       const char     *text2 = 0,
	       int             cbid = 0);

    ~X_tbutton (void);

    virtual void redraw (void);

    void set_col (unsigned long bg, XftColor *fg) { _bg = bg, _fg = fg; }
    void set_text (const char *txt1, const char *txt2);
    const char   *text1 (void) const { return _text1; }
    const char   *text2 (void) const { return _text2; }

private:

    unsigned long   _bg;
    XftColor       *_fg;
    int             _len1;
    int             _len2;
    char            _text1 [MAXLEN];
    char            _text2 [MAXLEN];
};


class X_ibutton : public X_button
{
public:

    enum { MAXLEN = 24 };

    X_ibutton (X_window       *parent,
	       X_callback     *callb,
	       X_button_style *style,
	       int             xp,
	       int             yp,
               XImage         *image,
	       int             cbid = 0);

    ~X_ibutton (void);

    virtual void redraw (void);

    void set_col (unsigned long bg, unsigned long fg) { _bg = bg, _fg = fg; }
    void set_image (XImage *image) { _image = image; }
    const XImage *image (void) const { return _image; }

private:

    unsigned long   _bg;
    unsigned long   _fg;
    XImage         *_image;
};


class X_pbutton : public X_button
{
public:

    X_pbutton (X_window       *parent,
	       X_callback     *callb,
	       X_button_style *style,
	       int             xp,
	       int             yp,
               Pixmap          map0,
               Pixmap          map1,
 	       int             cbid = 0);
 
    ~X_pbutton (void);

    virtual void set_stat (bool stat);

    const Pixmap map0 (void) const { return _map0; }
    const Pixmap map1 (void) const { return _map1; }

private:
   
    Pixmap _map0;
    Pixmap _map1;
};


// -------------------------------------------------------------------------------------------


class X_scroll : public X_window, public X_callback
{
public:

    enum { ENDGAP = 1, PLUSMIN = 2, MIDGAP = 4, ARROWS = 8 };
    enum { MOVE, STOP, C1LU, C3LU, C1RD, C3RD, B1LU, B3LU, B1RD, B3RD, B1PL, B3PL, B1MI, B3MI };

    void redraw (void);

    X_scroll (X_window       *parent,
	      X_callback     *callb,
	      X_button_style *style,
	      int xp,
	      int yp,
              int xs,
              int ys,
              int opts);

    ~X_scroll (void);

    float offs (void) const { return _offs; }
    float frac (void) const { return _frac; }
    void  geom (float offs, float frac, int size = 0);
    void  move (float drel);

private:

    void handle_event (XEvent *E);
    void bpress (XButtonEvent *E);
    void brelease (XButtonEvent *E);
    void bmotion (XMotionEvent *E);
    void expose (XExposeEvent *E);

    virtual void handle_callb (int type, X_window *W, XEvent *E);

    const X_button_style *_style;
    X_callback           *_callb;
    int             _xs;
    int             _ys;
    int             _km;
    int             _k0;
    int             _dk;
    int             _dw;
    int             _zz;
    float           _offs;
    float           _frac;
};


// -------------------------------------------------------------------------------------------


class X_textln_style
{

public:

    XftFont   *font;
    struct 
    {
	struct
	{
	    unsigned long bgnd;
	    XftColor     *text;
	} normal;
	struct 
	{ 
	    unsigned long bgnd;
	    XftColor     *text;
	    unsigned long line; 
	} focus;
	struct 
	{ 
	    unsigned long bgnd;
	    unsigned long lite;
	    unsigned long dark;
	} shadow;
    } color;
};


// -------------------------------------------------------------------------------------------


class X_textln : public X_window
{
public:

    X_textln (X_window         *parent, 
	      X_textln_style   *style,
	      int xp,
              int yp,
	      int xs,
              int ys,
              const char *text,
              int xal);

    ~X_textln (void);

    int base (void) const { return _y0; };

private:

    void handle_event (XEvent *xe);
    void expose (XExposeEvent *e);

    X_textln_style  *_style;
    const char      *_text;
    int              _len;
    int              _x0;
    int              _y0;
};


// -------------------------------------------------------------------------------------------


class X_textip : public X_window, public X_linked
{
public:

    enum { KEY, BUT, MODIF };

    X_textip (X_window         *parent, 
	      X_callback       *callb,
	      X_textln_style   *style,
	      int xp,
              int yp,
	      int xs,
              int ys,
	      int max,
	      X_linked    *back = 0,
	      X_linked    *forw = 0);

    ~X_textip (void);

    void set_text (const char *txt);
    void set_color (unsigned long bg, XftColor *fg);
    void set_align (int xal);
    void enable (void);
    void clear_modified (void) { _flags |= CLEAR; }
    void callb_modified (void) { if (_callb) _flags |= CALLB; }

    const char *text (void) { _txt [_i1] = 0; return (char *) _txt; }
    int  base (void) const { return _y0; };
    int  key  (void) const { return _key; }
    X_linked *forw (void) const { return _forw; }
    X_linked *back (void) const { return _back; } 

private:

    enum { FOCUS = 1, CLEAR = 2, CALLB = 4, RIGHT = 8, CENTER = 16 };

    int  test_utf8 (int k, const unsigned char *t);
    void insert (int k, const unsigned char *t);
    void go_sol (void);
    void go_lt (void);
    void del_rt (void);
    void go_eol (void);
    void go_rt (void);
    void del_eol (void);
    void del_all (void);
    void del_lt (void);
    void spkey (XKeyEvent *e);
    void cbkey (int k, XKeyEvent *e);

    void handle_event (XEvent *xe);
    void expose (XExposeEvent *e);
    void bpress (XButtonEvent *e);
    void keypress (XKeyEvent *e);
    void paste (XSelectionEvent *e);
    void setfocus (XFocusChangeEvent *e);
    void remfocus (XFocusChangeEvent *e);
    void update (bool f);
    void redraw (void);
    void xorcursor (void);
    void checkclear (void);
    void checkcallb (void);
    void select (void);
    int  findindex (int x);
    int  textwidth (int i, int n);
 
    X_callback     *_callb;
    X_textln_style *_style;
    unsigned long   _bg;
    XftColor       *_fg;
    int             _flags;
    int             _xs;
    int             _ys;
    unsigned char  *_txt;
    int             _max;
    int             _key;
    int             _ic;
    int             _i1;
    int             _xc;
    int             _x0;
    int             _x1;
    int             _y0;

    static           XIC _xic;
    static unsigned char _utf8mark [6];
};


// -------------------------------------------------------------------------------------------


class X_enip_item
{
public:
    int         _code;
    const char *_text;
};


class X_enumip : public X_window, public X_linked
{
public:

    enum { KEY, BUT };

    X_enumip (X_window *parent, 
	      X_callback      *callb,
	      X_textln_style  *style,
	      int xp,
              int yp,
	      int xs,
              int ys,
	      X_enip_item     *list,
	      X_linked    *back = 0,
	      X_linked    *forw = 0);

    ~X_enumip (void);

    void set_ind (int ind);
    int  base (void) const { return _y0; };
    int  key  (void) const { return _key; }
    int  ind  (void) const { return _ind; }

    virtual void handle_event (XEvent *xe);

private:

    void spkey (XKeyEvent *e);
    void cbkey (int k);

    void expose (XExposeEvent *e);
    void bpress (XButtonEvent *e);
    void keypress (XKeyEvent *e);
    void setfocus (XFocusChangeEvent *e);
    void remfocus (XFocusChangeEvent *e);
    void redraw (void);
    int  textwidth (int i, int n);

    virtual void select (void);

    X_callback     *_callb;
    X_textln_style *_style;
    X_enip_item    *_list;
    unsigned long   _bg;
    XftColor       *_fg;
    char           *_txt;
    int             _ind;
    int             _max;
    int             _len;
    int             _foc;
    int             _key;
    int             _xs;
    int             _ys;
    int             _x0;
    int             _y0;

    static           XIC _xic;
    static unsigned char _utf8mark [6];
};


// -------------------------------------------------------------------------------------------


class X_menuwin_style
{
public:

    enum { PLAIN = 0, BORDER = 1, RAISED = 2, PDOWN = 256 };

    XftFont       *font;
    int            step;
    int            type;
    struct 
    {
	XftColor     *bg;
	XftColor     *fg [4];
	XftColor     *mm;
	struct 
	{ 
	    unsigned long bgnd;
	    unsigned long lite;
	    unsigned long dark;
	} shadow;
    } color;
};


class X_menuwin_item
{
public:

    enum { COLMASK = 3, TITLE = 0x010000, SPACE = 0x020000, LAST = 0x040000, MARK = 0x080000, MASKED = 0x100000 };

    const char      *_text;
    int              _bits;
    X_menuwin_item  *_link;  
};



class X_menuwin : public X_window
{
public:

    enum { OPEN, SEL, CAN };
    enum { MAXITEM = 32, DL = 6, DR = 6 };

    X_menuwin (X_window         *parent,
	       X_callback       *callb,
	       X_menuwin_style  *style,
	       int xp,
	       int yp,
	       X_menuwin_item   *items); 

    ~X_menuwin (void);

    void show (void);
    void hide (void);
    void set_mask (unsigned m);
    void clr_mask (unsigned m);
    int xs (void) { return _xs; }
    int ys (void) { return _ys; }
    int selection (void) { return _isel; }


private:

    void handle_event (XEvent *E);
    void expose (XExposeEvent *E);
    void enter (XEnterWindowEvent *E);
    void leave (XLeaveWindowEvent *E);
    void motion (XPointerMovedEvent *E);
    void bpress (XButtonEvent *E);
    void brelse (XButtonEvent *E);
    void redraw (bool clear);
    void drawit (int k, bool inv);
    void select (int k);
    int  findit (int x, int y);

    const X_menuwin_style *_style;
    X_callback            *_callb;
    X_menuwin_item        *_items;
    int             _xs;
    int             _ys;
    int             _nit;
    int             _isel;
    int             _open;
    int             _ypos [MAXITEM];
    int             _tlen [MAXITEM];
};


// -------------------------------------------------------------------------------------------


class X_scale_style
{

public:

    enum { SEGMAX = 20 };

    int             marg;
    int             nseg;
    int             pix  [SEGMAX + 1];
    float           val  [SEGMAX + 1];
    const char     *text [SEGMAX + 1];
    XftFont        *font;
    unsigned long   bg;
    XftColor       *fg;

    void   set_tick (int i, int p, float v, const char *t) { pix [i] = p; val [i] = v; text [i] = t; }
    void   set_text (int i, const char *t) { text [i] = t; }
    int    pixlen (void) const { return pix [0] + pix [nseg] + 1; }
    int    length (void) const { return 2 * marg + pixlen (); }
    int    limit (int i);
    int    calcpix (float x);
    float  calcval (int p);
};


class X_hscale : public X_window
{
public:

    X_hscale (X_window       *parent,
	      X_scale_style  *style,
	      int xp,
	      int yp,
	      int h,
	      int d = 0);

private:

    void handle_event (XEvent *xe);
    void expose (XExposeEvent *e);

    X_scale_style  *_style;
    int      _h;
    int      _d;
};


class X_vscale : public X_window
{
public:

    X_vscale (X_window       *parent,
	      X_scale_style  *style,
	      int xp,
	      int yp,
	      int w, 
	      int d = 0);

private:

    void handle_event (XEvent *xe);
    void expose (XExposeEvent *e);

    X_scale_style  *_style;
    int      _w;
    int      _d;
};


// -------------------------------------------------------------------------------------------


class X_meter_style
{
public:

    unsigned long bg;
    struct
    {
        unsigned long c0;
        unsigned long c1;
        unsigned long mk; 
    } off;
    struct
    {
        unsigned long c0;
        unsigned long c1;
        unsigned long mk; 
    } act;
};


class X_meter : public X_window
{
public:

    X_meter (X_window *parent, X_meter_style *style, X_scale_style *scale, int xp, int yp, int xs, int ys);
 
    void handle_event (XEvent *xe);
    void set_val (float v);
    void set_ref (float v);

    friend class X_hmeter;
    friend class X_vmeter;

private:

    void expose (XExposeEvent *e);
    void plotsect (int i0, int i1, int imin, int imax, int cl, int cr);
    void plotmarks (int a, int b, unsigned long c);

    virtual void psect (GC g, int a, int b) = 0;
    virtual void pmark (GC g, int a) = 0;

    X_meter_style  *_style;
    X_scale_style  *_scale;

    int      _x;
    int      _y;
    int      _d;
    int      _i;
    int      _s;
    int      _d0;
    int      _d1;
    int      _d2;
};


class X_hmeter : public X_meter
{
public:

    X_hmeter (X_window *parent, X_meter_style *style, X_scale_style *scale, int xp, int yp, int h);

private:

    virtual void psect (GC g, int a, int b);
    virtual void pmark (GC g, int a);
};


class X_vmeter : public X_meter
{
public:

    X_vmeter (X_window *parent, X_meter_style *style, X_scale_style *scale, int xp, int yp, int w);

private:

    virtual void psect (GC g, int a, int b);
    virtual void pmark (GC g, int a);
};


// -------------------------------------------------------------------------------------------


class X_slider_style
{
public:

  unsigned long bg;
  unsigned long lite;
  unsigned long dark;
  unsigned long knob;
  unsigned long mark;
  int           w;
  int           h;
};


class X_slider : public X_window
{
public:

    enum { MOVE, STOP };

    X_slider (X_window *parent,
              X_callback *callb,
              X_slider_style *style,
              X_scale_style *scale, 
              int xp, int yp,
              int xs, int ys,
              int cbid = 0);
 
    virtual void set_val (float v) = 0;
    virtual void set_color (unsigned long knob, unsigned long mark) = 0;
    float get_val (void) { return _scale->calcval (_i); }
    int   cbid (void) const { return _cbid; }

    friend class X_hslider;
    friend class X_vslider;

private:

    void brelease (XButtonEvent *E);

    X_callback      *_callb;
    X_slider_style  *_style;
    X_scale_style   *_scale;
 
    int      _x;
    int      _y;
    int      _i;
    int      _d;
    int      _cbid;
    unsigned long  _knob;
    unsigned long  _mark; 
};


class X_hslider : public X_slider
{
public:

    X_hslider (X_window *parent,
               X_callback *callb,
               X_slider_style *style,
               X_scale_style *scale,
               int xp, int yp,
               int h,
               int cbid);

    virtual void handle_event (XEvent *E);
    virtual void set_color (unsigned long knob, unsigned long mark);
    virtual void set_val (float v);

private:

    bool domove (int i);
    void plines (void);
    void plknob (int i);
    void erase (int i);
};


class X_vslider : public X_slider
{
public:

    X_vslider (X_window *parent,
               X_callback *callb,
               X_slider_style *style,
               X_scale_style *scale,
               int xp, int yp,
               int w,
               int cbid);

    virtual void handle_event (XEvent *E);
    virtual void set_color (unsigned long knob, unsigned long mark);
    virtual void set_val (float v);

private:

    bool domove (int i);
    void plines (void);
    void plknob (int i);
    void erase (int i);
};


// -------------------------------------------------------------------------------------------


class X_mclist_style
{
public:

    unsigned long   bg;
    XftColor       *fg [4];
    XftFont        *font;
    int             dy;
};


class X_mclist : public X_window
{
public:

    enum { OPEN, SEL, CAN };

    X_mclist (X_window        *parent,
	      X_callback      *callb,
	      X_mclist_style  *style,
	      int             xp,
	      int             yp,
	      int             xs,
	      int             ys,
	      int             max_item,
	      int             max_char);

    ~X_mclist (void);

    void reset (void) { _n_item = _n_char = 0; _sel = 0; }
    int  item (const char *txt, int col, int len = 0);  
    void sort (void);
    void show (void);
    void move (int offs);
    void resize (int xs, int ys);
    void forw (void);
    void back (void);

    int  xs (void) const { return _xs; }
    int  ys (void) const { return _ys; }
    int  span (void) const { return _span; }
    int  offs (void) const { return _offs; }
    const char *ptr (void) const { return _ptr [_sel]; }
    int col (void) const { return _col [_sel]; }
    int len (void) const { return _len [_sel]; } 

private:

    void handle_event (XEvent *xE);
    void expose (XExposeEvent *E);
    void bpress (XButtonEvent *E);
    void update (int xx, int yy, int ww, int hh);

    X_mclist_style  *_style;
    X_callback      *_callb;
    int      _max_item;
    int      _max_char;
    int      _n_item;
    int      _n_char;
    int      _xs;
    int      _ys;
    int      _offs;
    int      _span;
    int      _nrow;
    int      _nclm;
    int      _sel;
    char    *_buff;
    char   **_ptr;
    short   *_len; 
    short   *_ext;
    short   *_col;
    int     *_ind;
    int     *_dxc;
};


// -------------------------------------------------------------------------------------------


#endif

