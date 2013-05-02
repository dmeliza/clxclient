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


#include <string.h>
#include <ctype.h>
#include "clxclient.h"


XrmOptionDescRec X_resman::defopt [X_resman::NDEFOPT] =
{
    {(char *)("-display"),          (char *)(".display"),          XrmoptionSepArg,  NULL },
    {(char *)("-name"),             (char *)(".name"),             XrmoptionSepArg,  NULL },
    {(char *)("-title"),            (char *)(".title"),            XrmoptionSepArg,  NULL },
    {(char *)("-iconic"),           (char *)("*iconic"),           XrmoptionNoArg,   (char *)("on") },
    {(char *)("-mono"),             (char *)(".mono"),             XrmoptionNoArg,   (char *)("on") },
    {(char *)("-gray"),             (char *)(".gray"),             XrmoptionNoArg,   (char *)("on") },
    {(char *)("-geometry"),         (char *)("*geometry"),         XrmoptionSepArg,  NULL },
    {(char *)("-background"),       (char *)("*background"),       XrmoptionSepArg,  NULL },
    {(char *)("-bg"),               (char *)("*background"),       XrmoptionSepArg,  NULL },
    {(char *)("-foreground"),       (char *)("*foreground"),       XrmoptionSepArg,  NULL },
    {(char *)("-fg"),               (char *)("*foreground"),       XrmoptionSepArg,  NULL },
    {(char *)("-font"),             (char *)("*font"),             XrmoptionSepArg,  NULL },
    {(char *)("-fn"),               (char *)("*font"),             XrmoptionSepArg,  NULL },
};



X_resman::X_resman (void) :
    _xrmdata (NULL)
{
}


X_resman::~X_resman (void)
{
}


void X_resman::init (int *argc, char *argv [], char *name,
		     XrmOptionDescRec *opt, int nopt)
{
    XrmDatabase cmd_db = NULL;  
    XrmDatabase def_db = NULL;
    XrmDatabase app_db = NULL;

    char     line [256];
    int      ac;
    char   **av;

    _home = getenv ("HOME");
    if (_home == 0) _home = "";

    // set default name and class
    strncpy (_rname, name, 64);
    strncpy (_rclas, name, 64);
    _rclas [0] = toupper (_rclas [0]);

    // Scan command line args for -name and -class
    ac = *argc;
    av =  argv;
    while (++av, --ac > 0)
    {
	if (! strcmp (*av, "-name")  && ac > 1) strncpy (_rname, av [1], 64);
	if (! strcmp (*av, "-class") && ac > 1) strncpy (_rclas, av [1], 64);
    }

    // Initialise X resource manager
    XrmInitialize ();

    // Read user default options. 
    sprintf (line, "%s/.Xdefaults", _home);
    def_db = XrmGetFileDatabase (line);

    // Read options from application options file or system wide config file
    sprintf (line, "%s/.%src", _home, _rname);
    app_db = XrmGetFileDatabase (line);
    if (! app_db)
    {
        sprintf (line, "/etc/%s.conf", _rname);
        app_db = XrmGetFileDatabase (line);
    }    

    // Scan command line for options.
    if (opt && nopt) XrmParseCommand (&cmd_db, opt, nopt, _rname, argc, argv);

    // Merge all Xrm databases
    XrmMergeDatabases (def_db, &_xrmdata);
    XrmMergeDatabases (app_db, &_xrmdata);
    XrmMergeDatabases (cmd_db, &_xrmdata);
}


const char *X_resman::get (const char *res, const char *def) const
{
    char       r1 [128];
    char       r2 [128];
    char       *t [20];
    XrmValue   v;

    strcpy (r1, _rname);
    strcat (r1, res);
    strcpy (r2, _rclas);
    strcat (r2, res);
    return XrmGetResource (_xrmdata, r1, r2, t, &v) ? (char *) v.addr : def;
}


int X_resman::getb (const char *res, int def) const
{
    const char *val;

    val = get (res, 0);
    if (val) return ! (    strcmp (val, "on")
  		        && strcmp (val, "true")
			&& strcmp (val, "On")
			&& strcmp (val, "True"));
    else return def;
}


void X_resman::geometry (const char *res, int xd, int yd, int bd, int &xp, int &yp, int &xs, int &ys)
{
    const char *g;
    int   f, xp1, yp1;
    unsigned int xs1, ys1;

    g = get (res, NULL);
    if (g)
    {
	f = XParseGeometry (g, &xp1, &yp1, &xs1, &ys1);
 
	if (f & WidthValue)  xs = xs1;
	if (f & HeightValue) ys = ys1;

	if (f & XValue)
	{
	    if (f & XNegative) xp1 += xd - xs - bd;
	    xp = xp1;
	}
	if (f & YValue)
	{
	    if (f & YNegative) yp1 += yd - ys - bd;
	    yp = yp1;
	}
    }
}

