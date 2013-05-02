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


#include <sys/poll.h>
#include "clxclient.h"


X_handler::X_handler (X_display *disp, Edest *dest, int ipid) :
    H_thread (dest, ipid),
    _conn (ConnectionNumber (disp-> dpy ()))
{
    thr_start (SCHED_OTHER, 0, 0x00010000);
}


X_handler::~X_handler (void)
{
    sepuku ();
}


extern "C" int x_handler_error (Display *dpy, XErrorEvent *err)
{
    char s [256];

    XGetErrorText (dpy, err->error_code, s, 256);
    fprintf (stderr, "X_handler::error () %s\n", s);
    return 0;
}


void X_handler::thr_main (void)
{
    struct pollfd pfd [1];

    XSetErrorHandler (x_handler_error);

    get_event (); 
    while (1)
    {
	pfd [0].fd = _conn;
	pfd [0].events = POLLIN | POLLERR;
        if (poll (pfd, 1, -1) < 0)
	{
	    if (errno == EINTR) continue;
	    perror ("X_handler: poll()");
	}
	if (pfd [0].revents & POLLIN) reply ();
	else return;
	get_event (); 
    }
}
