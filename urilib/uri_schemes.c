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
  int scheme_length = 0;

  {
    char* start = object->pool;
    char* end;
    while(*start && isspace(*start))
      start++;
    end = start;
    while(*end && ( isalnum(*end) || *end == '+' || *end == '.' || *end == '-'))
      end++;
    if(*end != '\0' && end > start && *end == ':') {
      scheme = start;
      scheme_length = end - start;
    }
  }

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
      uri_error(object->pool_size * 2, "unknown scheme %.*s found in url %s\n", scheme_length, scheme, object->pool);
      return -1;
    }
  }

  {
    int ret = object->desc->parse(object);
    if(ret == 0)
      object->info |= URI_INFO_PARSED;
    return ret;
  }
}

int uri_cannonicalize(uri_t* object, int flag)
{
  if((object->info & URI_INFO_PARSED) == 0)
    return URI_NOT_CANNONICAL;

  if(object->desc) {
    int ret;
    static char* tmp = 0;
    static int tmp_size = 0;
    int tmp_length = 0;
    uri_t object_save;

    if(flag == URI_CANNONICALIZE_TEST) {
      /*
       * Save pointers to fields
       */
      object_save = *object;
    }

    if(object->info & URI_INFO_CANNONICAL) return URI_CANNONICAL;
    /* 
     * multiply by 3 to get the maximum expand length of the URI (all
     * chars converted to %xx sequences.
     */
    static_alloc(&tmp, &tmp_size, object->pool_size * 3);
    
    /*
     * Effectively cannonicalize the URL and put the result in tmp
     */
    ret = object->desc->cannonicalize(object, tmp, &tmp_length, flag);

    if(flag == URI_CANNONICALIZE_TEST) {
      *object = object_save;
    } else if(ret == URI_CANNONICAL) {
      /*
       * Relocate the normalized object.
       */
      if(object->pool_size < tmp_length) {
	object->pool = (char*)srealloc(object->pool, tmp_length);
	object->pool_size = tmp_length;
      }
      memcpy(object->pool, tmp, tmp_length);
#define reloc(w) if(object->w) object->w = object->pool + (object->w - tmp)
      reloc(scheme);
      reloc(host);
      reloc(port);
      reloc(path);
      reloc(params);
      reloc(query);
      reloc(frag);
      reloc(user);
      reloc(passwd);
#undef reloc

      object->info |= URI_INFO_CANNONICAL;
    }
  
    return ret;
    
  } else {
    uri_error(0, "cannonicalize: no desc\n");
    return URI_NOT_CANNONICAL;
  }
}

void uri_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  if(object->desc)
    object->desc->string(object, stringp, string_sizep, flags);
  else
    uri_error(0, "uri_string: no desc\n");
}

