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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <uri.h>
#include <uri_schemes.h>
#include <uri_scheme_generic.h>


uri_scheme_desc_t uri_scheme_https_desc = {
  /* parse */		uri_scheme_generic_parse,
  /* cannonicalize */	uri_scheme_generic_cannonicalize,
  /* string */		uri_scheme_generic_string,
  /* specs */		uri_scheme_generic_specs,
  /* port_int */	443,
  /* port_char */	"443"
};
