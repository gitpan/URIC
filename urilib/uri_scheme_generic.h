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
#ifndef _uri_scheme_generic_h
#define _uri_scheme_generic_h

int uri_scheme_generic_parse(uri_t* object);
int uri_scheme_generic_cannonicalize(uri_t* object, char* tmp, int* tmp_lengthp, int flag);
void uri_scheme_generic_string(uri_t* object, char** stringp, int* string_sizep, int flags);

/*
 * If the SPEC_<part> bit is set, the char may appear unencoded in
 * this part of the URI.
 *
 * If the SPEC_E<part> bit is set, the char must appear encoded in
 * this part of the URI.
 *
 * If both SPEC_<part> and SPEC_E<part> bits are set, the character is
 * left untouched.
 */
#define SPEC_SCHEME	0x0001
#define SPEC_NETLOC	0x0002
#define SPEC_PATH	0x0004
#define SPEC_QUERY	0x0008
#define SPEC_PARAMS	0x0010
#define SPEC_TAG	0x0020
#define SPEC_AUTH	0x0040

#define SPEC_ESCHEME	0x00010000
#define SPEC_ENETLOC	0x00020000
#define SPEC_EPATH	0x00040000
#define SPEC_EQUERY	0x00080000
#define SPEC_EPARAMS	0x00100000
#define SPEC_ETAG	0x00200000
#define SPEC_EAUTH	0x00400000

/* Must be escaped whatever the context is (except scheme and netloc). */
#define SPEC_ESC	SPEC_EPATH|SPEC_EQUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH
/* Allowed unescaped except scheme and netloc. */
#define SPEC_NOR	SPEC_PATH|SPEC_QUERY|SPEC_PARAMS|SPEC_TAG|SPEC_AUTH

/* Can be left unescaped whatever the context is. */
#define SPEC_ALNUM	SPEC_SCHEME|SPEC_NETLOC|SPEC_PATH|SPEC_QUERY|SPEC_PARAMS|SPEC_TAG|SPEC_AUTH

extern int uri_scheme_generic_specs[];

extern uri_scheme_desc_t uri_scheme_generic_desc;

#endif /* _uri_scheme_generic_h */
