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
#include <uri_parse.h>

#include "uri_schemes_hash.c"

/*
 * Description objects declarations
 */
#include "uri_schemes_desc.h"

/*
 * scheme to description mapping table
 */
uri_scheme_desc_t* scheme2desc[MAX_HASH_VALUE] = {
#include "uri_schemes_desc.c"
};

int uri_parse(uri_t* object)
{
  char* scheme;
  int scheme_length;
  int ret;

  uri_parse_find_scheme(object->pool, &scheme, &scheme_length);
  if(uri_scheme_switch(object, scheme, scheme_length) < 0)
    return -1;

  ret = object->desc->parse(object);

  if(ret == 0)
    object->info |= URI_INFO_PARSED;

  return ret;
}

int uri_scheme_switch(uri_t* object, char* scheme, int scheme_length)
{
  /*
   * Scheme omitted, must be relative URL, switch to generic
   */
  if(scheme_length == 0) {
    object->desc = &uri_scheme_generic_desc;
  } else {
    char tmp[MAX_WORD_LENGTH + 1];
    int too_long = scheme_length > MAX_WORD_LENGTH;

    if(!too_long) {
      strncpy(tmp, scheme, scheme_length);
      tmp[MAX_WORD_LENGTH] = '\0';
      strlower(tmp, scheme_length);
    }
    /*
     * Branch according to scheme
     */
    if(!too_long && uri_scheme_exists(tmp, scheme_length)) {
      object->desc = scheme2desc[uri_scheme_hash(tmp, scheme_length)];
    } else {
      /*
       * Strictly speaking we should accept any scheme. In practice it
       * mostly leads to schemes with typos being handled silently.
       */
      if((mode.flag & URI_MODE_URI_STRICT) || (mode.flag & URI_MODE_URI_STRICT_SCHEME)) {
	object->desc = &uri_scheme_generic_desc;
      } else {
	uri_error(uri_estimate_pool_size(object) * 2, "unknown scheme %.*s found in url %s\n", scheme_length, scheme, object->pool);
	return -1;
      }
    }
  }
  return 0;
}

int uri_cannonicalize(uri_t* object)
{
  if((object->info & URI_INFO_PARSED) == 0)
    return URI_NOT_CANNONICAL;

  if(!object->desc) {
    uri_error(0, "cannonicalize: no desc\n");
    return URI_NOT_CANNONICAL;
  }

  if(uri_cannonicalp(object))
    return URI_CANNONICAL;
  
  {
    int ret;
    static char* tmp = 0;
    static int tmp_size = 0;
    uri_t object_tmp;

    if(object->info & URI_INFO_CANNONICAL) return URI_CANNONICAL;
    /* 
     * multiply by 3 to get the maximum expand length of the URI (all
     * chars converted to %xx sequences.
     */
    static_alloc(&tmp, &tmp_size, uri_estimate_pool_size(object) * 3);
    
    /*
     * Effectively cannonicalize the URL and put the result in tmp
     */
    object_tmp = *object;
    ret = object->desc->cannonicalize(object, &object_tmp, tmp, &object_tmp.pool_size);

    /*
     * Normalize case
     */
    if(object_tmp.scheme) strlower(object_tmp.scheme, -1);

    /*
     * Sanity checks for relative URIs
     */
    if(object_tmp.info & URI_INFO_RELATIVE) {
#define check(w) \
  if(object_tmp.w != 0) \
  { \
    uri_error(uri_estimate_pool_size(object), "cannonicalize: in %s, " #w " cannot be set in relative uri\n", uri_uri(object)); \
    return URI_NOT_CANNONICAL; \
  }
      check(port);
      check(passwd);
      check(user);
    }
    /*
     * Cleanup domain name variations
     */
    if(object_tmp.host) {
      int length = strlen(object_tmp.host);
      /*
       * Kill trailing dots
       */
      while(length >= 1 && object_tmp.host[length - 1] == '.') {
	length--;
	object_tmp.host[length] = '\0';
      }
      if(length == 0) {
	uri_error(uri_estimate_pool_size(object), "uri_cannonicalize: %s has null netloc\n", uri_uri(object));
	return URI_NOT_CANNONICAL;
      }
    }

    /*
     * Reduce path name, if not relative URI.
     */
    if(!(object_tmp.info & URI_INFO_RELATIVE) && object_tmp.path)
      minimal_path(object_tmp.path, -1);

    if(ret == URI_CANNONICAL) {
      /*
       * Override current object.
       */
      if(mode.flag & URI_MODE_CANNONICAL) {
	uri_copy(object, &object_tmp);
	object->info |= URI_INFO_CANNONICAL;
      } else {
	/*
	 * Store cannonical object in object->cannonical
	 */
	if(object->cannonical == 0)
	  object->cannonical = uri_clone(&object_tmp);
	else
	  uri_copy(object->cannonical, &object_tmp);
	object->info |= URI_INFO_CANNONICAL_OK;
      }
    } else {
      object->info &= ~(URI_INFO_CANNONICAL_OK|URI_INFO_CANNONICAL);
    }

    return ret;
  }
}

void uri_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  if(object->desc)
    object->desc->string(object, stringp, string_sizep, flags);
  else
    uri_error(0, "uri_string: no desc\n");
}

/*
 * Access functions
 */

int uri_info(uri_t* object)
{
  uri_consistent(object);
  return object->info;
}

#define D(n) \
  char* uri_##n(uri_t* object) \
  { \
    return object->desc->n(object); \
  }

D(scheme)
D(host)
D(port)
D(path)
D(params)
D(query)
D(frag)
D(user)
D(passwd)
D(netloc)
D(auth)
D(all_path)

#undef D

/*
 * Modifier functions
 */

void uri_info_set(uri_t* object, int value)
{
  object->info = value;
}

#define D(n) \
  void uri_##n##_set(uri_t* object, char* value) \
  { \
    object->desc->n##_set(object, value); \
  }

D(scheme)
D(host)
D(port)
D(path)
D(params)
D(query)
D(frag)
D(user)
D(passwd)

#undef D

char* uri_auth_netloc(uri_t* object)
{
  static char* tmp = 0;
  static int tmp_size = 0;
  char* auth = uri_auth(object);
  char* netloc = uri_netloc(object);

  /*
   * + 1 for nul + 1 for @
   */
  static_alloc(&tmp, &tmp_size, strlen(auth) + strlen(netloc) + 1 + 1);

  tmp[0] = '\0';

  if(object->host && auth[0] != '\0') {
    strcat(tmp, auth);
    strcat(tmp, "@");
  }
  strcat(tmp, netloc);

  return tmp;
}

