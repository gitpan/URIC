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

#include <uri_escapes_generic.h>

/*
 * Does not implement the multiple port specification :80,81.
 * I wonder what kind of server support it, anyway.
 */
int uri_scheme_generic_parse(uri_t* object)
{
  return uri_parse_generic(object, 0);
}

int uri_scheme_generic_cannonicalize(uri_t* original, uri_t* object, char* tmp, int* tmp_sizep)
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
    normalize(port, SPEC_NETLOC);
    normalize(path, SPEC_PATH);
    normalize(params, SPEC_PARAMS);
    normalize(query, SPEC_QUERY);
    normalize(frag, SPEC_FRAG);
    normalize(user, SPEC_AUTH);
    normalize(passwd, SPEC_AUTH);
#undef normalize
  }

  if(object->host) strlower(object->host, -1);

  *tmp_sizep = tmp_size;

  return URI_CANNONICAL;
}

void uri_scheme_generic_string(uri_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;

  static_alloc(stringp, string_sizep, uri_estimate_pool_size(object));

  string = *stringp;

  string[0] = '\0';
  if(object->scheme) {
    strcat(string, object->scheme);
    strcat(string, ":");
  }
  if(object->host || object->user || object->passwd || object->port) {
    strcat(string, (flags & URI_STRING_FURI_STYLE ? "/" : "//"));
    strcat(string, uri_auth_netloc(object));
  }
  if(!(object->info & URI_INFO_RELATIVE) ||
     !(object->info & URI_INFO_RELATIVE_PATH))
    strcat(string, "/");
  if(flags & URI_STRING_ROBOTS_STYLE) {
    strcat(string, "robots.txt");
  } else {
    if(object->path) strcat(string, object->path);
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
  if(_relative == 0) _relative = uri_alloc_1();

  if(uri_realloc(_relative, relative_string, relative_length) != URI_CANNONICAL)
    return 0;

  return uri_abs_1(base, _relative);
}

static uri_t* absolute = 0;

/*
 * draft-fielding-uri-syntax-04.txt section 5.2
 */
uri_t* uri_abs_1(uri_t* base, uri_t* relative)
{
  static char* path = 0;
  static int path_size = 0;

  if(!(relative->info & URI_INFO_RELATIVE))
    return relative;

  if(absolute == 0) absolute = uri_alloc_1();

  {
    int absolute_length = uri_estimate_pool_size(base) + uri_estimate_pool_size(relative);
    if(absolute->pool_size < absolute_length)
      static_alloc(&absolute->pool, &absolute->pool_size, absolute_length + 1);
  }
  uri_clear(absolute);

  /*
   * 5.2 3) Accept scheme in relative URL. 
   * In strict mode, if the scheme is present it's absolute
   * In compatible mode, if the scheme is present and different from 
   * base scheme, it's absolute.
   */
  if((mode.flag & URI_MODE_URI_STRICT) ||
     (mode.flag & URI_MODE_URI_STRICT_SCHEME)) {
    if(relative->scheme)
      return relative;
  } else {
    if(relative->scheme && strcasecmp(relative->scheme, base->scheme))
      return relative;
  }

  /*
   * Relative is empty, absolute equal base
   */
  if((relative->path == 0 || relative->path[0] == '\0') &&
     relative->host == 0 && relative->query == 0 && relative->params == 0 &&
     relative->frag == 0)
    return base;

  /*
   * 5.2 2) Reference to the current document
   */
  if((relative->path == 0 || relative->path[0] == '\0') &&
     relative->host == 0 && relative->query == 0 && relative->params == 0) {
    absolute->scheme = base->scheme;
    absolute->user = base->user;
    absolute->passwd = base->passwd;
    absolute->host = base->host;
    absolute->port = base->port;
    absolute->path = base->path;
    absolute->params = base->params;
    absolute->query = base->query;
    absolute->frag = relative->frag;
  } else {
    /*
     * When we reach this part we know that the 'relative' object is
     * not absolute, i.e. either host or scheme were omitted.
     */

    if(mode.flag & URI_MODE_URI_STRICT)
      absolute->scheme = base->scheme;
    else
      absolute->scheme = relative->scheme ? relative->scheme : base->scheme;
      

    /*
     * 5.2 4) Host set implies relative URL only needed scheme from base
     */
    if(relative->host) {
      absolute->host = relative->host;
      absolute->port = relative->port;
      absolute->user = relative->user;
      absolute->passwd = relative->passwd;
      absolute->path = relative->path;
      absolute->params = relative->params;
      absolute->query = relative->query;
      absolute->frag = relative->frag;
    } else {
      absolute->host = base->host;
      absolute->port = base->port;
      absolute->user = base->user;
      absolute->passwd = base->passwd;
      /*
       * 5.2 6) Build the new absolute path by merging relative and base.
       */
      static_alloc(&path, &path_size,
		   (relative->path ? strlen(relative->path) : 0) +
		   (base->path ? strlen(base->path) : 0) + 1);
      path[0] = '\0';
      if(relative->info & URI_INFO_RELATIVE_PATH) {
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
	if(relative->path == 0 || relative->path[0] == '\0') {
	  if((mode.flag & URI_MODE_URI_STRICT) == 0 || (relative->params == 0 && relative->query == 0))
	    if(base->path) strcpy(path, base->path);
	  /*
	   * If the relative uri path is relative, append to dirname of 
	   * base path and reduce . and ..
	   */
	} else {
	  strcat(path, relative->path);
	  minimal_path(path, -1);
	}
      } else {
	/*
	 * 5.2 5) Relative URL contains absolute path
	 */
	strcpy(path, relative->path);
	minimal_path(path, -1);
      }
      absolute->path = path;
      /*
       * In strict mode all params+query are taken from the relative
       * URL regardless. 
       * In compatible mode params+query are kept from base if path
       * is not set in relative, query is kept from base if params is
       * not set in relative.
       */
      if((mode.flag & URI_MODE_URI_STRICT) ||
	 (relative->path && relative->path[0] != '\0')) {
	absolute->params = relative->params;
	absolute->query = relative->query;
      } else {
	if(relative->params) {
	  absolute->params = relative->params;
	  absolute->query = relative->query;
	} else {
	  absolute->params = base->params;
	  absolute->query = relative->query ? relative->query : base->query;
	}
      }
      absolute->frag = relative->frag;
    }
  }

  /*
   * 5.2 7) Recombine components from base and relative into absolute
   */
  {
    char* tmp;
    char* p = absolute->pool;
    int length;
#define reloc(w) \
    if(absolute->w) { \
      strcpy(p, absolute->w); \
      absolute->w = p; \
      p += strlen(absolute->w) + 1; \
    }
    reloc(scheme);
    reloc(user);
    reloc(passwd);
    reloc(host);
    reloc(port);
    reloc(path);
    reloc(params);
    reloc(query);
    reloc(frag);
#undef reloc
  }
  uri_scheme_switch(absolute, absolute->scheme, strlen(absolute->scheme));
  return absolute;
}

/*
 * Access functions
 */

#define D(n) char* uri_scheme_generic_##n(uri_t* object) { return object->n ? object->n : ""; }
D(scheme)
D(host)
     /* D(port) */
D(path)
D(params)
D(query)
D(frag)
D(user)
D(passwd)
#undef D

char* uri_scheme_generic_netloc(uri_t* object)
{
  static char* netloc = 0;
  static int netloc_size = 0;

  {
    int length = 10 +
      (object->host ? strlen(object->host) : 0) +
      (object->port ? strlen(object->port) : 0);
    
    static_alloc(&netloc, &netloc_size, length);
  }

  netloc[0] = '\0';
  if(object->host) {
    /*
     * Include port if exists and not default port of scheme.
     */
    if(object->port &&
       (object->desc->port_char == 0 || strcmp(object->desc->port_char, object->port))) {
      sprintf(netloc, "%s:%s", object->host, object->port);
    } else {
      strcpy(netloc, object->host);
    }
  }

  return netloc;
}

char* uri_scheme_generic_auth(uri_t* object)
{
  static char* auth = 0;
  static int auth_size = 0;

  {
    int length = 10 +
      (object->user ? strlen(object->user) : 0) +
      (object->passwd ? strlen(object->passwd) : 0);
    
    static_alloc(&auth, &auth_size, length);
  }

  if(object->user && object->passwd) {
    sprintf(auth, "%s:%s", object->user, object->passwd);
  } else if(object->user) {
    strcpy(auth, object->user);
  } else {
    auth[0] = '\0';
  }

  return auth;
}

char* uri_scheme_generic_all_path(uri_t* object)
{
  static char* path = 0;
  static int path_size = 0;

  {
    int length = 10 +
      (object->path ? strlen(object->path) : 0) +
      (object->params ? strlen(object->params) : 0) +
      (object->query ? strlen(object->query) : 0);
    
    static_alloc(&path, &path_size, length);
  }

  path[0] = '\0';

  if(!(object->info & URI_INFO_RELATIVE) ||
     !(object->info & URI_INFO_RELATIVE_PATH))
    strcat(path, "/");
  if(object->path) strcat(path, object->path);
  if(object->params) {
    strcat(path, ";");
    strcat(path, object->params);
  }
  if(object->query) {
    strcat(path, "?");
    strcat(path, object->query);
  }

  return path;
}

char* uri_scheme_generic_port(uri_t* object)
{
  if(object->port) return object->port;
  if(object->desc && object->desc->port_int > 0) return object->desc->port_char;
  return "";
}

/*
 * Set structure fields. 
 * Warning! The modified object may be inconsistent after such a 
 * modification (URL_RELATIVE flag may be set and URL now absolute, for
 * instance). uri_consistent must be called after a set of direct field
 * modifications.
 */

#define B() \
     /* Empty string is same as no string */ \
     if(value && strlen(value) == 0) \
       value = 0

#define C(n,f) \
     B(); \
     /* Free previously allocated space. */ \
     if(object->info & URI_INFO_M_##f) \
       free(object->n); \
     /* Invalidate cache, field not allocated anymore. */ \
     object->info &= ~(URI_INFO_CACHE_MASK|URI_INFO_M_##f); \
     /* Malloc if field not null and mode is always allocate. */ \
     if((mode.flag & URI_MODE_FIELD_MALLOC) && value) { \
       object->n = strdup(value); \
       object->info |= URI_INFO_M_##f; \
     } else \
       object->n = value

#define D(n,f) \
  void uri_scheme_generic_##n##_set(uri_t* object, char* value)\
  { \
    C(n,f); \
  }

void uri_scheme_or_host_changed(uri_t* object)
{
  if(object->info & URI_INFO_RELATIVE) {
    /*
     * Object is no more relative : lose the distinction between
     * relative and absolute path that only have a meaning when
     * in a relative URL. A previously relative path will therefore
     * become absolute but it's the responsibility of the caller
     * to take care of that.
     */
    if(object->scheme && object->host) {
      object->info &= ~(URI_INFO_RELATIVE|URI_INFO_RELATIVE_PATH);
    }
  } else {
    if(!object->scheme || !object->host) {
      /* 
       * Object is no more absolute. Since the path is always
       * absolute in an absolute URL, it stays absolute in the
       * new relative URL. If a relative path is intended, it
       * must be set with uri_path_set.
       */
      object->info |= URI_INFO_RELATIVE;
      object->info &= ~(URI_INFO_RELATIVE_PATH);
    }
  }
}
    
void uri_scheme_generic_scheme_set(uri_t* object, char* value)
{
  int changed;

  B();
  
  changed = ((object->scheme == 0 && value != 0) ||
	     (object->scheme != 0 && value == 0) ||
	     (object->scheme && strcasecmp(object->scheme, value))
	     );

  C(scheme,SCHEME);

  if(changed) {
    uri_scheme_or_host_changed(object);
    if(object->scheme)
      uri_scheme_switch(object, object->scheme, strlen(object->scheme));
  }
}

void uri_scheme_generic_host_set(uri_t* object, char* value)
{
  int changed;

  B();

  changed = ((object->host == 0 && value != 0) ||
	     (object->host != 0 && value == 0) ||
	     (object->host && strcasecmp(object->host, value))
	     );

  C(host, HOST);

  if(changed)
    uri_scheme_or_host_changed(object);
}

D(port,PORT)

void uri_scheme_generic_path_set(uri_t* object, char* value)
{
  B();

  if(value) {
    if(object->info & URI_INFO_RELATIVE) {
      if(*value == '/') {
	object->info &= ~URI_INFO_RELATIVE_PATH;
	value++;
      } else 
	object->info |= URI_INFO_RELATIVE_PATH;
    }
  } else {
    object->info &= ~URI_INFO_RELATIVE_PATH;
  }
  C(path,PATH);
}

D(params,PARAMS)
D(query,QUERY)
D(frag,FRAG)
D(user,USER)
D(passwd,PASSWD)

#undef B
#undef C
#undef D


uri_scheme_desc_t uri_scheme_generic_desc = {
  /* parse */		uri_scheme_generic_parse,
  /* cannonicalize */	uri_scheme_generic_cannonicalize,
  /* string */		uri_scheme_generic_string,
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
