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
#ifndef _uri_h
#define _uri_h

/*
 * Possible values of the uri library mode
 */
/*
 * URI are transformed in cannonical form as soon as they are
 * parsed.
 */
#define URI_MODE_CANNONICAL	0x01
/*
 * When an error occur, print the error on stderr
 */
#define URI_MODE_ERROR_STDERR	0x02

/*
 * If uri_mode is not set the following parameters are in use
 * just as if uri_mode(URI_MODE_FLAG_DEFAULT)
 * was called.
 */
#define URI_MODE_FLAG_DEFAULT		(URI_MODE_CANNONICAL|URI_MODE_ERROR_STDERR)

/*
 * Set library wide mode
 */
void uri_mode(int flag);
/*
 * True if flag is set in library wide mode
 */
int uri_modep(int flag);

/*
 * Possible values of the info field in the uri_t structure.
 */
/*
 * Set if URI is cannonical
 */
#define URI_INFO_CANNONICAL	0x0001
/*
 * Set if the private uri field is allocated.
 */
#define URI_INFO_URI		0x0002
/*
 * Set if the private furi field is allocated.
 */
#define URI_INFO_FURI		0x0004
/*
 * Set if the URI is relative.
 */
#define URI_INFO_RELATIVE	0x0008
/*
 * Set if the URI is relative and the path is relative.
 */
#define URI_INFO_RELATIVE_PATH	0x0010
/*
 * Set if the URI is a null string.
 */
#define URI_INFO_EMPTY		0x0020
/*
 * Set if the URI has been parsed successfully
 */
#define URI_INFO_PARSED		0x0040
/*
 * Set if the URI is a robots.txt URI
 */
#define URI_INFO_ROBOTS		0x0400

/*
 * Actual definition in uri_schemes.h
 */
struct uri_scheme_desc;

/*
 * Splitted URI information.
 */
typedef struct uri {
  /* Public fields */
  int info;		/* Information */
  char* scheme;		/* URI scheme (http, ftp...) */
  char* host;		/* hostname part (www.foo.com) */
  char* port;		/* port part if any (www.foo.com:8080 => 8080) */
  char* path;		/* path portion without params and query */
  char* params;		/* params part (/foo;dir/bar => foo) */
  char* query;		/* query part (/foo?bar=val => bar=val) */
  char* frag;		/* frag part (/foo#part => part) */
  char* user;		/* user part (http://user:pass@www.foo.com => user) */
  char* passwd;		/* user part (http://user:pass@www.foo.com => pass) */

  /* Private fields */
  char* pool;
  int pool_size;
  char* furi;
  int furi_size;
  char* uri;
  int uri_size;
  char* robots;
  int robots_size;
  struct uri_scheme_desc* desc;
} uri_t;

/*
 * Transform URI string in uri_t
 */
uri_t* uri_alloc(char* uri, int uri_length);
int uri_realloc(uri_t* object, char* uri, int uri_length);
/*
 * Release object allocated by uri_alloc
 */
void uri_free(uri_t* object);

/*
 * Access structure fields
 */
int uri_info(uri_t* object);
char* uri_scheme(uri_t* object);
char* uri_host(uri_t* object);
char* uri_port(uri_t* object);
char* uri_path(uri_t* object);
char* uri_params(uri_t* object);
char* uri_query(uri_t* object);
char* uri_frag(uri_t* object);
char* uri_user(uri_t* object);
char* uri_passwd(uri_t* object);
char* uri_netloc(uri_t* object);
char* uri_auth(uri_t* object);
char* uri_all_path(uri_t* object);

/*
 * Copy uri_t objects
 */
void uri_copy(uri_t* to, uri_t* from);
/*
 * Clear all fields, does not deallocate
 */
void uri_clear(uri_t* object);
/*
 * Show fields on stderr.
 */
void uri_dump(uri_t* object);

/*
 * Convert structure into string.
 */
/*
 * File equivalent of an URI
 */
char* uri_furi(uri_t* object);
/*
 * URI string
 */
char* uri_uri(uri_t* object);

/*
 * Convinience functions
 */
/*
 * Return static structure instead of malloc'd structure.
 * Must *not* be freed.
 */
uri_t* uri2object(char* uri, int uri_length);

/*
 * Possible values of the flag argument of uri_furi_string.
/*
 * Prepend the content of the WLROOT environment variable
 * to the FURI.
 */
#define URI_FURI_REAL_PATH	1
/*
 * Do not prepend anything to the FURI.
 */
#define URI_FURI_NOP		0
/*
 * Equivalent to uri_furi(uri_object(uri, strlen(uri)))
 */
char* uri_furi_string(char* uri, int uri_length, int flag);

/*
 * Possible values of the flag argument of uri_cannonicalize_string.
 */
/*
 * Return a FURI
 */
#define URI_STRING_FURI_STYLE		0x01
/*
 * Return an URI
 */
#define URI_STRING_URI_STYLE		0x02
/*
 * Return the robots.txt URI corresponding to this URI
 */
#define URI_STRING_ROBOTS_STYLE		0x04
/*
 * Omit the frag in the returned string.
 */
#define URI_STRING_URI_NOHASH_STYLE	0x08
/*
 * Cannonicalize the given uri and return it in the chosen form.
 */
char* uri_cannonicalize_string(char* uri, int uri_length, int flag);

/*
 * The last error message
 */
extern char* uri_errstr;

/*
 * Functions of uri_schemes.c that branch according to scheme.
 */

/*
 * Return values of uri_cannonicalize
 */
#define URI_NOT_CANNONICAL	-1
#define URI_CANNONICAL		0

/*
 * Possible values of the info field in the uri_t structure.
 */
/*
 * Only try to see if cannonicalization is possible, do not transform object
 */
#define URI_CANNONICALIZE_TEST		0x01
/*
 * Transform object in cannonical form
 */
#define URI_CANNONICALIZE_TRANSFORM	0x02
/*
 * Implement cannonicalization of URI
 */
int uri_cannonicalize(uri_t* object, int flag);
#define uri_cannonicalp(o) ((o)->info & URI_INFO_CANNONICAL)

/*
 * Parse string pointed by pool and fill fields.
 */
int uri_parse(uri_t* object);

/*
 * The following functions are not available for all scheme. Their
 * implementation may be found in the corresponding uri_scheme_???.c file.
 */

/*
 * GENERIC specific
 */

/*
 * Transform relative URI in absolute URI according to base.
 */
uri_t* uri_abs(uri_t* base, char* relative_string, int relative_length);
uri_t* uri_abs_1(uri_t* base, uri_t* relative);

/*
 * HTTP specific
 */

/*
 * robots.txt URI corresponding to this URI.
 */
char* uri_robots(uri_t* object);

#endif /* _uri_h */
