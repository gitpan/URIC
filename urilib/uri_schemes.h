/*
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
#ifndef _uri_schemes_h
#define _uri_schemes_h

typedef int (*uri_parse_func_t)(uri_t* object);
typedef int (*uri_cannonicalize_func_t)(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep);
typedef void (*uri_string_func_t)(uri_t* object, char** stringp, int* string_sizep, int flags);
typedef char* (*uri_access_func_t)(uri_t* object); 
typedef void (*uri_set_func_t)(uri_t* object, char* value); 

typedef struct uri_scheme_desc {
  uri_parse_func_t		parse;
  uri_cannonicalize_func_t	cannonicalize;
  uri_string_func_t		string;
  uri_access_func_t	        scheme;
  uri_access_func_t	        host;
  uri_access_func_t	        port;
  uri_access_func_t	        path;
  uri_access_func_t	        params;
  uri_access_func_t	        query;
  uri_access_func_t	        frag;
  uri_access_func_t	        user;
  uri_access_func_t	        passwd;
  uri_access_func_t	        netloc;
  uri_access_func_t	        auth;
  uri_access_func_t	        all_path;
  uri_set_func_t	        scheme_set;
  uri_set_func_t	        host_set;
  uri_set_func_t	        port_set;
  uri_set_func_t	        path_set;
  uri_set_func_t	        params_set;
  uri_set_func_t	        query_set;
  uri_set_func_t	        frag_set;
  uri_set_func_t	        user_set;
  uri_set_func_t	        passwd_set;
  int*				specs;
  int				port_int;
  char*				port_char;
} uri_scheme_desc_t;

#endif /* _uri_schemes_h */
