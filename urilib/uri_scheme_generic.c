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

/*
 * Does not implement the multiple port specification :80,81.
 * I wonder what kind of server support it, anyway.
 */
int uri_scheme_generic_parse(uri_t* object)
{
  char* p;

  /*
   *  This parsing code is based on
   *   draft-ietf-uri-relative-uri-06.txt Section 2.4
   */
  /* 2.4.1 frag */
  if(p = strrchr(object->pool, '#')) {
    object->frag = p + 1;
    *p = '\0';
  }
  p = object->pool;
  /* 2.4.2 scheme */
  {
    char* start = p;
    char* end;
    while(*start && isspace(*start))
      start++;
    end = start;
    while(*end && ( isalnum(*end) || *end == '+' || *end == '.' || *end == '-'))
      end++;
    if(*end != '\0' && end > start && *end == ':') {
      object->scheme = start;
      *end = '\0';
      p = end + 1;
    }
  }
  /*
   * 2.4.3 netloc
   * Never bother to find the netloc if there is no scheme.
   * It may even lead to errors if done (//foo.bar/dir/file.html for instance)
   */
  if(object->scheme) {
    char* start = p;
    char* end;

    if(start[0] == '/' && start[1] == '/') {
      /*
       * Tolerate /// 
       */
      while(*start && *start == '/')
	start++;
      end = start;
      while(*end && *end != '/')
	end++;
      p = *end ? end + 1 : end;
      *end = '\0';
      /*
       * Decode authentication information.
       */
      {
	char* auth_end;
	if(auth_end = strchr(start, '@')) {
	  char* auth_start = start;
	  *auth_end = '\0';
	  start = auth_end + 1;

	  if(object->passwd = strchr(auth_start, ':')) {
	    *object->passwd++ = '\0';
	  }
	  object->user = auth_start;
	}
      }
      if(end > start) {
	char* tmp;
	*end = '\0';
	object->host = start;
	if(tmp = strrchr(start, ':')) {
	  *tmp = '\0';
	  object->port = tmp + 1;
	}
      }
    }
  }

  if(!object->scheme || !object->host) {
    object->info |= URI_INFO_RELATIVE;
  }

  /* 2.4.4 query */
  object->query = strchr(p, '?');
  if(object->query) {
    *object->query = '\0';
    object->query++;
  }
  /* 2.4.5 query */
  object->params = strchr(p, ';');
  if(object->params) {
    *object->params = '\0';
    object->params++;
  }
  /* Multiple / in path are always mistakes */
  {
    char* read = p;
    char* write = p;
    int slash = 0;
    if((object->info & URI_INFO_RELATIVE) &&
       *p != '/') {
      object->info |= URI_INFO_RELATIVE_PATH;
    }
    while(*read && *read == '/')
      read++;
    while(*read) {
      if(slash && *read == '/') {
	read++;
	slash = 1;
      } else {
	*write++ = *read++;
	slash = 0;
      }
    }
    *write = '\0';
  }
  object->path = p;

  if(object->scheme == 0 &&
     object->host == 0 &&
     object->port == 0 &&
     object->path[0] == '\0' &&
     object->params == 0 &&
     object->query == 0 &&
     object->frag == 0 &&
     object->user == 0 &&
     object->passwd == 0) {
    object->info |= URI_INFO_EMPTY;
  }
  return 0;
}


/*
 * Refer to rfc 1738 for constraints imposed to URIs. Some
 * details are derived from actual use of URI which may differ
 * slightly from the specifications.
 */

int uri_scheme_generic_specs[256] = {
/*  00 nul  01 soh   02 stx  03 etx   04 eot  05 enq   06 ack  07 bel   */
  0,        SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  08 bs   09 ht    0a nl   0b vt    0c np   0d cr    0e so   0f si    */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  10 dle  11 dc1   12 dc2  13 dc3   14 dc4  15 nak   16 syn  17 etb   */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  18 can  19 em    1a sub  1b esc   1c fs   1d gs    1e rs   1f us    */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  20 sp   21 !     22 "    23 #     24 $    25 %     26 &    27 '     */
  SPEC_ESC, SPEC_NOR, SPEC_ESC, SPEC_ESC,
  SPEC_NOR,
  SPEC_ESC,
  SPEC_EPATH|SPEC_QUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
  SPEC_ESC,
/*  28 (    29 )     2a *    2b +     2c ,    2d -     2e .    2f /     */
  SPEC_NOR,
  SPEC_NOR,
  SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_PATH|SPEC_EPATH|SPEC_EQUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
/*  30 0    31 1     32 2    33 3     34 4    35 5     36 6    37 7     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  38 8    39 9     3a :    3b ;     3c <    3d =     3e >    3f ?     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC, SPEC_ESC,
  SPEC_ESC,
  SPEC_EPATH|SPEC_QUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
  SPEC_ESC,
  SPEC_ESC,
/*  40 @    41 A     42 B    43 C     44 D    45 E     46 F    47 G     */
  SPEC_ESC,   SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  48 H    49 I     4a J    4b K     4c L    4d M     4e N    4f O     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  50 P    51 Q     52 R    53 S     54 T    55 U     56 V    57 W     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  58 X    59 Y     5a Z    5b [     5c \    5d ]     5e ^    5f _     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC,
  SPEC_ESC,   SPEC_ESC,   SPEC_ESC,   SPEC_NOR,
/*  60 `    61 a     62 b    63 c     64 d    65 e     66 f    67 g     */
  SPEC_ESC,   SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  68 h    69 i     6a j    6b k     6c l    6d m     6e n    6f o     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  70 p    71 q     72 r    73 s     74 t    75 u     76 v    77 w     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  78 x    79 y     7a z    7b {     7c |    7d }     7e ~    7f del   */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC,
  SPEC_ESC,   SPEC_ESC,   SPEC_NOR,   SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, 
};

int uri_scheme_generic_cannonicalize(uri_t* object, char* tmp, int* tmp_lengthp, int flag)
{
  int tmp_length = 0;

  {
    int length;
#define normalize(w,s) if(object->w) { length = cannonicalize_component(object, object->w, tmp + tmp_length, (s), #s); object->w = tmp + tmp_length; tmp_length += length; if(length < 0) return URI_NOT_CANNONICAL; }
    /*
     * WARNING! keep the following call in the same order as in the 
     * original string.
     */
    normalize(scheme, SPEC_SCHEME);
    normalize(host, SPEC_NETLOC);
    normalize(port, SPEC_NETLOC);
    normalize(path, SPEC_PATH);
    normalize(params, SPEC_PARAMS);
    normalize(query, SPEC_QUERY);
    normalize(frag, SPEC_TAG);
    normalize(user, SPEC_AUTH);
    normalize(passwd, SPEC_AUTH);
#undef normalize
  }
  
  /*
   * Normalize case
   */
  if(object->scheme) strlower(object->scheme, -1);
  if(object->host) strlower(object->host, -1);

  /*
   * Sanity checks for relative URIs
   */
  if(object->info & URI_INFO_RELATIVE) {
#define check(w) if(object->w != 0) { uri_error(object->pool_size, "cannonicalize: in %s, " #w " cannot be set in relative uri, ignored\n", uri_uri(object)); object->w = 0; object->info &= ~URI_INFO_URI; }
    check(port);
    check(passwd);
    check(user);
  }
  /*
   * Cleanup domain name variations
   */
  if(object->host) {
    int length = strlen(object->host);
    /*
     * Kill trailing dots
     */
    while(length >= 1 && object->host[length - 1] == '.') {
      length--;
      object->host[length] = '\0';
    }
    if(length == 0) {
      uri_error(object->pool_size, "uri_cannonicalize: %s has null netloc\n", uri_uri(object));
      return URI_NOT_CANNONICAL;
    }
  }

  if(flag != URI_CANNONICALIZE_TEST) {
    /*
     * Reduce path name, if not relative URI.
     */
    if(!(object->info & URI_INFO_RELATIVE)) 
      minimal_path(object->path, -1);
  }

  *tmp_lengthp = tmp_length;

  return URI_CANNONICAL;
}

void uri_scheme_generic_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  /*
   * + 16 is more than enough for separators & all 
   */
  static_alloc(stringp, string_sizep, object->pool_size + 16);

  string = *stringp;

  string[0] = '\0';
  if(object->scheme) {
    strcat(string, object->scheme);
    strcat(string, ":");
  }
  if(object->host || object->user || object->passwd || object->port) {
    strcat(string, (flags & URI_STRING_FURI_STYLE ? "/" : "//"));
    if(object->user) {
      strcat(string, object->user);
      if(object->passwd) {
	strcat(string, ":");
	strcat(string, object->passwd);
      }
      strcat(string, "@");
    }
    if(object->host) strcat(string, object->host);
    if(object->port) {
      strcat(string, ":");
      strcat(string, object->port);
    }
  }
  if(!(object->info & URI_INFO_RELATIVE) ||
     !(object->info & URI_INFO_RELATIVE_PATH))
    strcat(string, "/");
  if(flags & URI_STRING_ROBOTS_STYLE) {
    strcat(string, "robots.txt");
  } else {
    strcat(string, object->path);
    if(object->params) {
      strcat(string, ";");
      strcat(string, object->params);
    }
    if(object->query) {
      strcat(string, "?");
      strcat(string, object->query);
    }
    if(object->frag && (flags & URI_STRING_URI_STYLE) && !(flags & URI_STRING_URI_NOHASH_STYLE)) {
      strcat(string, "#");
      strcat(string, object->frag);
    }
  }
}

/*
 * Relative URLs functions
 */

/*
 * draft-fielding-uri-syntax-03 describes relative URIs.
 * Some modifications have been done to accomodate effective net usage:
 * . http:/french/index.html is a relative URI (draft-fielding-uri-syntax-03
 *   says it is an absolute)
 *   provided that the scheme is the same as the scheme of the base uri.
 */

static uri_t* _relative = 0;

uri_t* uri_abs(uri_t* base, char* relative_string, int relative_length)
{
  if(_relative == 0) {
#define DUMMY "http://www.dummy.org/dir/file"
    _relative = uri_alloc(DUMMY, strlen(DUMMY));
#undef DUMMY
  }
  if(uri_realloc(_relative, relative_string, relative_length) != URI_CANNONICAL)
    return 0;

  return uri_abs_1(base, _relative);
}

static uri_t* absolute = 0;

uri_t* uri_abs_1(uri_t* base, uri_t* relative)
{
  static char* path = 0;
  static int path_size = 0;

  int no_relative_path = 0;

  if(relative->info & URI_INFO_EMPTY)
    return base;
  if(!(relative->info & URI_INFO_RELATIVE))
    return relative;

  if(absolute == 0) {
#define DUMMY "http://www.dummy.org/dir/file"
    absolute = uri_alloc(DUMMY, strlen(DUMMY));
#undef DUMMY
  }

  if(absolute->pool_size < base->pool_size + relative->pool_size) {
    absolute->pool_size = base->pool_size + relative->pool_size;
    absolute->pool = (char*)srealloc(absolute->pool, absolute->pool_size);
  }
  uri_clear(absolute);

  /*
   * Build the new absolute path by merging relative and base.
   */
  {
    static_alloc(&path, &path_size,
		 (relative->path ? strlen(relative->path) : 0) +
		 (base->path ? strlen(base->path) : 0) + 1);
    path[0] = '\0';

    /* 
     * Move the base path to path, striping the last file name.
     */
    {
      char* last_slash;
      if(base->path && (last_slash = strrchr(base->path, '/'))) {
	/* + 1 means that we want to keep the trailing / */
	int length = last_slash - base->path + 1;
	memcpy(path, base->path, length);
	path[length] = '\0';
      }
    }
    
    /*
     * If the relative uri path is null or empty, keep the base path if any
     */
    if(relative->path == 0 ||
       relative->path[0] == '\0') {
      if(base->path) strcpy(path, base->path);
      no_relative_path = 1;
    /*
     * If the relative uri path is absolute, override base
     */
    } else if(!(relative->info & URI_INFO_RELATIVE_PATH)) {
      strcpy(path, relative->path);
      minimal_path(path, -1);
    /*
     * If the relative uri path is relative, append to dirname of base path
     * and reduce . and ..
     */
    } else {
      strcat(path, relative->path);
      minimal_path(path, -1);
    }
  }

  /*
   * Recombine components from base and relative into absolute
   */
  {
    char* tmp;
    char* p = absolute->pool;
    int length;
#define merge(w) \
    if(tmp) { \
      strcpy(p, tmp); \
      absolute->w = p; \
      p += strlen(tmp) + 1; \
    } 
    tmp = base->scheme ? base->scheme : relative->scheme;
    merge(scheme);
    tmp = base->host;
    merge(host);
    tmp = base->port;
    merge(port);
    tmp = path;
    if(path[0] != '\0') {
      merge(path);
    } else {
      absolute->path = p;
      *p++ = '\0';
    }
    if(no_relative_path) {
      tmp = relative->params ? relative->params : base->params;
      merge(params);
      tmp = relative->query ? relative->query : base->query;
      merge(query);
      tmp = relative->frag ? relative->frag : base->frag;
      merge(frag);
    } else {
      tmp = relative->params;
      merge(params);
      tmp = relative->query;
      merge(query);
      tmp = relative->frag;
      merge(frag);
    }
    tmp = relative->user ? relative->user : base->user;
    merge(user);
    tmp = relative->passwd ? relative->passwd : base->passwd;
    merge(passwd);
  }
#undef merge
  absolute->info = URI_INFO_CANNONICAL;
  return absolute;
}

uri_scheme_desc_t uri_scheme_generic_desc = {
  /* parse */		uri_scheme_generic_parse,
  /* cannonicalize */	uri_scheme_generic_cannonicalize,
  /* string */		uri_scheme_generic_string,
  /* specs */		uri_scheme_generic_specs,
  /* port_int */	-1,
  /* port_char */	0
};
