/*
 *   Copyright (C) 1995, 1996, 1997, 1998
 *      Civil Engineering in Cyberspace
 *   Copyright (C) 1997, 1998
 *   	Free Software Foundation, Inc.
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2, or (at your option) any
 *   later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include <uri_util.h>
#include <uri.h>

char* uri_errstr = 0;
static int uri_errstr_size = 0;

#if STDC_HEADERS
void uri_error(int size_hint, char* fmt, ...)
#else
void uri_error(int size_hint, char* fmt, va_alist)
#endif
{
  va_list ap;

#if STDC_HEADERS
  va_start(ap, fmt);
#else
  va_start(ap);
#endif
  
  static_alloc(&uri_errstr, &uri_errstr_size, 512 + size_hint);
  vsprintf(uri_errstr, fmt, ap);
  if(uri_modep(URI_MODE_ERROR_STDERR))
    fprintf(stderr, "uri: %s", uri_errstr);

  va_end(ap);
}
