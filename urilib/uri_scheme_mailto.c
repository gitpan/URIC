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

int uri_scheme_mailto_parse(uri_t* object)
{
  char* p;

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

  /* user */
  {
    char* end = strchr(p, '@');
    if(end) {
      *end = '\0';
      object->user = p;
      p = end + 1;
    } else {
      uri_error(object->pool_size, "mailto_parse: missing @ in %s\n", object->pool);
      return -1;
    }
  }
  /* host */
  {
    object->host = p;
  }

  return 0;
}

int uri_scheme_mailto_cannonicalize(uri_t* object, char* tmp, int* tmp_lengthp, int flag)
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
    normalize(user, SPEC_AUTH);
    normalize(host, SPEC_NETLOC);
  }

  return URI_CANNONICAL;
}

void uri_scheme_mailto_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  /*
   * + 16 is more than enough for separators & all 
   */
  static_alloc(stringp, string_sizep, object->pool_size + 16);

  string = *stringp;

  string[0] = '\0';

  if(object->scheme && object->user && object->host) {
    sprintf(string, "%s:%s@%s", object->scheme, object->user, object->host);
  } else {
    uri_error(0, "mailto_string: unexpected empty field\n");
  }
}

uri_scheme_desc_t uri_scheme_mailto_desc = {
  /* parse */		uri_scheme_mailto_parse,
  /* cannonicalize */	uri_scheme_mailto_cannonicalize,
  /* string */		uri_scheme_mailto_string,
  /* specs */		uri_scheme_generic_specs,
  /* port_int */	-1,
  /* port_char */	0
};
