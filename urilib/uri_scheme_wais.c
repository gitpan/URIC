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

char* uri_parse_wais_path(uri_t* object, char* p);

int uri_scheme_wais_parse(uri_t* object)
{
  char* p = object->pool;

  p = uri_parse_scheme(object, p, 0);
  p = uri_parse_netloc(object, p, 0);
  p = uri_parse_query(object, p, 0);
  p = uri_parse_wais_path(object, p);

  if(!object->host || strlen(object->host) == 0) {
    uri_error(uri_estimate_pool_size(object), "wais_parse: empty host in %s\n", uri_uri(object));
    return -1;
  }

  if(object->query) {
    if(object->wais_wtype != 0 ||
       object->wais_wpath != 0) {
      uri_error(uri_estimate_pool_size(object), "wais_parse: unexpected wtype or wpath with query in %s\n", uri_uri(object));
      return -1;
    }
  } else if(object->wais_wtype) {
    if(object->wais_wtype == 0 || strlen(object->wais_wtype) == 0) {
      uri_error(uri_estimate_pool_size(object), "wais_parse: missing wtype in %s\n", uri_uri(object));
      return -1;
    }
    if(object->wais_wpath == 0 || strlen(object->wais_wpath) == 0) {
      uri_error(uri_estimate_pool_size(object), "wais_parse: missing wpath in %s\n", uri_uri(object));
      return -1;
    }
  }
  
  return 0;
}

char* uri_parse_wais_path(uri_t* object, char* p)
{
  char* start = p;

  while(*start && *start == '/')
    start++;

  /*
   * database
   */
  object->wais_database = start;
  
  {
    /*
     * wtype
     */
    char* tmp;
    if(tmp = strchr(start, '/')) {
      *tmp = '\0';
      start = tmp + 1;
      object->wais_wtype = start;
      /*
       * wpath
       */
      if(tmp = strchr(start, '/')) {
	*tmp = '\0';
	start = tmp + 1;
	object->wais_wpath = start;
      }
    }
  }

  return p;
}

int uri_scheme_wais_cannonicalize(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep)
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
    normalize(user, SPEC_NETLOC);
    normalize(passwd, SPEC_NETLOC);
    normalize(host, SPEC_NETLOC);
    normalize(wais_database, SPEC_TRANSP);
    normalize(query, SPEC_TRANSP);
    normalize(wais_wtype, SPEC_TRANSP);
    normalize(wais_wpath, SPEC_TRANSP);
  }

  if(object->host) strlower(object->host, -1);

  *tmp_sizep = tmp_size;

  return URI_CANNONICAL;
}

void uri_scheme_wais_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  static_alloc(stringp, string_sizep, uri_estimate_pool_size(object));

  string = *stringp;

  string[0] = '\0';

  if(object->scheme) {
    if(object->host) 
      sprintf(string, "%s://%s/", object->scheme, uri_auth_netloc(object));
    else
      sprintf(string, "%s:///", object->scheme);
  } else {
    uri_error(0, "wais_string: unexpected empty field\n");
    return;
  }

  if(object->wais_database != 0)
    strcat(string, object->wais_database);
  if(object->query != 0) {
    strcat(string, "?");
    strcat(string, object->query);
  } else {
    if(object->wais_wtype != 0) {
      strcat(string, "/");
      strcat(string, object->wais_wtype);
      if(object->wais_wpath != 0) {
	strcat(string, "/");
	strcat(string, object->wais_wpath);
      }
    }
  }
}

uri_scheme_desc_t uri_scheme_wais_desc = {
  /* parse */		uri_scheme_wais_parse,
  /* cannonicalize */	uri_scheme_wais_cannonicalize,
  /* string */		uri_scheme_wais_string,
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
  /* port_int */	210,
  /* port_char */	"210"
};

char* uri_wais_database(uri_t* object) { return uri_path(object); }
char* uri_wais_wtype(uri_t* object) { return uri_params(object); }
char* uri_wais_wpath(uri_t* object) { return uri_frag(object); }

void uri_wais_database_set(uri_t* object, char* value) { uri_path_set(object, value); }
void uri_wais_wtype_set(uri_t* object, char* value) { uri_params_set(object, value); }
void uri_wais_wpath_set(uri_t* object, char* value) { uri_frag_set(object, value); }
