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
#ifndef _uri_scheme_news_h
#define _uri_scheme_news_h

int uri_scheme_news_parse(uri_t* object);
int uri_scheme_news_cannonicalize(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep);
void uri_scheme_news_string(uri_t* object, char** stringp, int* string_sizep, int flags);

extern int uri_scheme_news_specs[];

#endif /* _uri_scheme_news_h */
