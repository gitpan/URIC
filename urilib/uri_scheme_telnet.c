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
#include <uri_schemes.h>
#include <uri_scheme_generic.h>

uri_scheme_desc_t uri_scheme_telnet_desc = {
  /* parse */		uri_scheme_generic_parse,
  /* cannonicalize */	uri_scheme_generic_cannonicalize,
  /* string */		uri_scheme_generic_string,
  /* scheme */		uri_scheme_generic_scheme,
  /* host */		uri_scheme_generic_host,
  /* port */		uri_scheme_generic_port,
  /* path */		uri_scheme_generic_path,
  /* params */		uri_scheme_generic_params,
  /* query */		uri_scheme_generic_query,
  /* frag */		uri_scheme_generic_frag,
  /* user */		uri_scheme_generic_user,
  /* passwd */		uri_scheme_generic_passwd,
  /* netloc */		uri_scheme_generic_netloc,
  /* passwd */		uri_scheme_generic_auth,
  /* all_path */	uri_scheme_generic_all_path,
  /* scheme_set */	uri_scheme_generic_scheme_set,
  /* host_set */	uri_scheme_generic_host_set,
  /* port_set */	uri_scheme_generic_port_set,
  /* path_set */	uri_scheme_generic_path_set,
  /* params_set */	uri_scheme_generic_params_set,
  /* query_set */	uri_scheme_generic_query_set,
  /* frag_set */	uri_scheme_generic_frag_set,
  /* user_set */	uri_scheme_generic_user_set,
  /* passwd_set */	uri_scheme_generic_passwd_set,
  /* specs */		uri_scheme_generic_specs,
  /* port_int */	23,
  /* port_char */	"23"
};
