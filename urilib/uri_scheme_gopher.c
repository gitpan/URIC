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

char* uri_parse_gopher_path(uri_t* object, char* p);

/*
 * Information from draft-murali-url-gopher.txt
 */
int uri_scheme_gopher_parse(uri_t* object)
{
  char* p = object->pool;

  p = uri_parse_scheme(object, p, 0);
  p = uri_parse_netloc(object, p, URI_SCHEME_GENERIC_PARSE_SKIP_AUTH);
  static_alloc(&object->gopher_tmp, &object->gopher_tmp_size, strlen(p));
  strcpy(object->gopher_tmp, uri_unescape(p));
  p = uri_parse_gopher_path(object, object->gopher_tmp);

  return 0;
}

char* uri_parse_gopher_path(uri_t* object, char* p)
{
  char* start = p;
  char c = *start;
  char buf[2];

  /*
   * Type
   */
#define GOPHER_TYPES "0123456789+IgT"
  if(!*start) c = '1';
  if(!strchr(GOPHER_TYPES, c)) {
    uri_error(uri_estimate_pool_size(object), "%c is not a know gopher type (" GOPHER_TYPES ") in uri %s\n", uri_uri(object));
    return p;
  }
  buf[0] = c; buf[1] = '\0';
  {
    int f = mode.flag;
    mode.flag |= URI_MODE_FIELD_MALLOC;
    uri_gopher_type_set(object, buf);
    mode.flag = f;
  }
  if(!*start) return p;
  start++;

  /*
   * Selector
   */
  object->gopher_selector = start;

  {
    /*
     * Search
     */
    char* tmp;
    if(tmp = strchr(start, '\t')) {
      *tmp = '\0';
      start = tmp + 1;
      object->gopher_search = start;
      /*
       * String
       */
      if(tmp = strchr(start, '\t')) {
	*tmp = '\0';
	start = tmp + 1;
	object->gopher_string = start;
      }
    }
  }
  
  return p;
}

int uri_scheme_gopher_cannonicalize(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep)
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
    normalize(host, SPEC_NETLOC);
    normalize(gopher_type, SPEC_PARAMS);
    normalize(gopher_selector, SPEC_PARAMS);
    normalize(gopher_search, SPEC_PARAMS);
    normalize(gopher_string, SPEC_PARAMS);
  }

  if(object->host) strlower(object->host, -1);

  *tmp_sizep = tmp_size;

  return URI_CANNONICAL;
}

void uri_scheme_gopher_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  static_alloc(stringp, string_sizep, uri_estimate_pool_size(object));

  string = *stringp;

  string[0] = '\0';

  if(object->scheme && object->host) {
    sprintf(string, "%s://%s/", object->scheme, uri_netloc(object));
  } else {
    uri_error(0, "gopher_string: unexpected empty field\n");
    return;
  }

  strcat(string, uri_path(object));
}

void uri_scheme_gopher_path_set(uri_t* object, char* value)
{
  if(value) {
    static_alloc(&object->gopher_tmp, &object->gopher_tmp_size, strlen(value));
    strcpy(object->gopher_tmp, uri_unescape(value));
  }

  uri_gopher_type_set(object, 0);
  uri_gopher_selector_set(object, 0);
  uri_gopher_search_set(object, 0);
  uri_gopher_string_set(object, 0);

  if(value)
    uri_parse_gopher_path(object, object->gopher_tmp);
}

char* uri_scheme_gopher_path(uri_t* object)
{
  static char* string = 0;
  static int string_size = 0;

  {
    int length = 5 +
      (object->gopher_selector ? strlen(object->gopher_selector) : 0) +
      (object->gopher_search ? strlen(object->gopher_search) : 0) +
      (object->gopher_string ? strlen(object->gopher_string) : 0);
    static_alloc(&string, &string_size, length);
  }

  string[0] = '\0';

  if(object->gopher_type == 0) {
    uri_error(0, "gopher_string: unexpected empty type\n");
    return 0;
  }
  if(object->gopher_selector || object->gopher_type[0] != '1') {
    strcat(string, object->gopher_type);
    if(object->gopher_selector) strcat(string, object->gopher_selector);
    if(object->gopher_search) {
      strcat(string, "%09");
      strcat(string, object->gopher_search);
    }
    if(object->gopher_string) {
      strcat(string, "%09");
      strcat(string, object->gopher_string);
    }
  }
  return string;
}

uri_scheme_desc_t uri_scheme_gopher_desc = {
  /* parse */		uri_scheme_gopher_parse,
  /* cannonicalize */	uri_scheme_gopher_cannonicalize,
  /* string */		uri_scheme_gopher_string,
  /* scheme */		uri_scheme_generic_scheme,
  /* host */		uri_scheme_generic_host,
  /* port */		uri_scheme_generic_port,
  /* path */		uri_scheme_gopher_path,
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
  /* path_set */	uri_scheme_gopher_path_set,
  /* params_set */	uri_scheme_generic_params_set,
  /* query_set */	uri_scheme_generic_query_set,
  /* frag_set */	uri_scheme_generic_frag_set,
  /* user_set */	uri_scheme_generic_user_set,
  /* passwd_set */	uri_scheme_generic_passwd_set,
  /* specs */		uri_scheme_generic_specs,
  /* port_int */	-1,
  /* port_char */	0
};

char* uri_gopher_type(uri_t* object) { return uri_user(object); }
char* uri_gopher_selector(uri_t* object) { return uri_params(object); }
char* uri_gopher_search(uri_t* object) { return uri_query(object); }
char* uri_gopher_string(uri_t* object) { return uri_frag(object); }

void uri_gopher_type_set(uri_t* object, char* value) { uri_user_set(object, value); }
void uri_gopher_selector_set(uri_t* object, char* value) { uri_params_set(object, value); }
void uri_gopher_search_set(uri_t* object, char* value) { uri_query_set(object, value); }
void uri_gopher_string_set(uri_t* object, char* value) { uri_frag_set(object, value); }
