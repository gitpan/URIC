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
#ifndef _uri_parse_h
#define _uri_parse_h

/*
 * Possible values of flags
 */
#define URI_SCHEME_GENERIC_PARSE_NONE			0x000000
#define URI_SCHEME_GENERIC_PARSE_SKIP_QUERY		0x000001
#define URI_SCHEME_GENERIC_PARSE_SKIP_AUTH		0x000002

int uri_parse_generic(uri_t* object, int flags);

void uri_parse_find_scheme(char* string, char** scheme, int* length);

char* uri_parse_scheme(uri_t* object, char* p, int flags);
char* uri_parse_netloc(uri_t* object, char* p, int flags);
char* uri_parse_query(uri_t* object, char* p, int flags);
char* uri_parse_params(uri_t* object, char* p, int flags);
char* uri_parse_frag(uri_t* object, char* p, int flags);
char* uri_parse_path(uri_t* object, char* p, int flags);

#endif /* _uri_parse_h */
