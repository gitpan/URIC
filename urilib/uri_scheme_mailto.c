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
#include <uri_private.h>
#include <uri_schemes.h>
#include <uri_scheme_generic.h>

int uri_scheme_mailto_parse(uri_t* object)
{
  char* p = object->pool;

  p = uri_parse_scheme(object, p, 0);

  /* user */
  object->user = p;
  {
    char* tmp;
    if(tmp = strchr(p, '@')) {
      *tmp = '\0';
      p = tmp + 1;
      object->host = p;
    }
  }

  return 0;
}

int uri_scheme_mailto_cannonicalize(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep)
{
  int tmp_size = 0;

  {
    int size;
#define normalize(w,s) \
  if(object->w) \
  { \
    size = cannonicalize_component(object, object->w, tmp + tmp_size, (s), #s); \
    if(size < 0) \
      return URI_NOT_CANNONICAL; \
    object->w = tmp + tmp_size; \
    tmp_size += size; \
  }
    /*
     * WARNING! keep the following call in the same order as in the 
     * original string.
     */
    normalize(scheme, SPEC_SCHEME);
    normalize(user, SPEC_AUTH);
    normalize(host, SPEC_NETLOC);
  }
  /*
   * Normalize case
   */
  if(object->host) strlower(object->host, -1);
  if(object->user) strlower(object->user, -1);

  *tmp_sizep = tmp_size;

  return URI_CANNONICAL;
}

void uri_scheme_mailto_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  static_alloc(stringp, string_sizep, uri_estimate_pool_size(object));

  string = *stringp;

  string[0] = '\0';

  if(object->scheme && object->user) {
    if(object->host) 
      sprintf(string, "%s:%s@%s", object->scheme, object->user, object->host);
    else
      sprintf(string, "%s:%s", object->scheme, object->user);
  } else {
    uri_error(0, "mailto_string: unexpected empty field\n");
  }
}

uri_scheme_desc_t uri_scheme_mailto_desc = {
  /* parse */		uri_scheme_mailto_parse,
  /* cannonicalize */	uri_scheme_mailto_cannonicalize,
  /* string */		uri_scheme_mailto_string,
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
  /* port_int */	-1,
  /* port_char */	0
};
