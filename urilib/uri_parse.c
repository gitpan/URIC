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

#include <uri_util.h>
#include <uri.h>
#include <uri_private.h>
#include <uri_parse.h>

int uri_parse_generic(uri_t* object, int flags)
{
  char* p = object->pool;

  p = uri_parse_frag(object, p, flags);
  p = uri_parse_scheme(object, p, flags);
  p = uri_parse_netloc(object, p, flags);
  p = uri_parse_query(object, p, flags);
  p = uri_parse_params(object, p, flags);
  p = uri_parse_path(object, p, flags);


  if(object->scheme == 0 &&
     object->host == 0 &&
     object->port == 0 &&
     (object->path == 0 || object->path[0] == '\0') &&
     object->params == 0 &&
     object->query == 0 &&
     object->frag == 0 &&
     object->user == 0 &&
     object->passwd == 0) {
    object->info |= URI_INFO_EMPTY;
  }
  return 0;
}

char* uri_parse_scheme(uri_t* object, char* p, int flags)
{
  char* scheme;
  int scheme_length;

  uri_parse_find_scheme(p, &scheme, &scheme_length);

  if(scheme_length > 0) {
    object->scheme = scheme;
    scheme[scheme_length] = '\0';
    p = scheme + scheme_length + 1;
    if(mode.flag & URI_MODE_LOWER_SCHEME) strlower(object->scheme, strlen(object->scheme));
  }

  return p;
}

void uri_parse_find_scheme(char* string, char** scheme, int* length)
{
  char* start = string;
  char* end;

  *scheme = 0;
  *length = 0;

  while(*start && isspace(*start))
    start++;
  end = start;
  while(*end && ( isalnum(*end) || *end == '+' || *end == '.' || *end == '-'))
    end++;
  if(*end != '\0' && end > start && *end == ':') {
    *scheme = start;
    *length = end - start;
  }
}

char* uri_parse_query(uri_t* object, char* p, int flags)
{
  if(!(flags & URI_SCHEME_GENERIC_PARSE_SKIP_QUERY)) {
    object->query = strchr(p, '?');
    if(object->query) {
      *object->query = '\0';
      object->query++;
    }
  }

  return p;
}

char* uri_parse_params(uri_t* object, char* p, int flags)
{
  object->params = strchr(p, ';');
  if(object->params) {
    *object->params = '\0';
    object->params++;
  }

  return p;
}

char* uri_parse_frag(uri_t* object, char* p, int flags)
{
  char* tmp;
  if(tmp = strrchr(p, '#')) {
    object->frag = tmp + 1;
    *tmp = '\0';
  }
  return p;
}

char* uri_parse_path(uri_t* object, char* p, int flags)
{
  /* Multiple / in path are always mistakes */
  {
    char* read = p;
    char* write = p;
    int slash = 0;
    if((object->info & URI_INFO_RELATIVE) &&
       !object->host &&
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

  return p;
}

char* uri_parse_netloc(uri_t* object, char* p, int flags)
{
  char* start = p;
  char* end;

  if(start[0] == '/' && start[1] == '/' && start[2] == '/') {
    /*
     * Null netloc as in http:///foo.html. Resume at /foo.html
     */
    p += 2;
  } else if(start[0] == '/' && start[1] == '/') {
    start += 2;
    end = start;
    while(*end && *end != '/')
      end++;
    p = *end ? end + 1 : end;
    *end = '\0';
    /*
     * Decode authentication information.
     */
    if((flags & URI_SCHEME_GENERIC_PARSE_SKIP_AUTH) == 0) {
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
	*tmp++ = '\0';
	if(*tmp)
	  object->port = tmp;
      }
    }
  }

  if(!object->scheme || !object->host)
    object->info |= URI_INFO_RELATIVE;

  return p;
}
