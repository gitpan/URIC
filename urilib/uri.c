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

static int verbose = 0;

static uri_t* _uri_object = 0;

uri_mode_t mode = {
  URI_MODE_FLAG_DEFAULT,
};

void uri_mode_set(int flag)
{
  mode.flag = flag;
}

int uri_mode()
{
  return mode.flag;
}

int uri_modep(int flag)
{
  return mode.flag & flag;
}

/*
 * uri_t allocation and deallocation
 */
uri_t* uri_object(char* uri, int uri_length)
{
  if(_uri_object) {
    int cannonical = uri_realloc(_uri_object, uri, uri_length);
    return cannonical == URI_CANNONICAL ? _uri_object : 0;
  } else
    return _uri_object = uri_alloc(uri, uri_length);
}

uri_t* uri_alloc_1() {
  uri_t* object = (uri_t*)smalloc(sizeof(uri_t));
  memset(object, '\0', sizeof(uri_t));
  return object;
}

uri_t* uri_alloc(char* uri, int uri_length)
{
  uri_t* object = uri_alloc_1();

  static_alloc(&object->pool, &object->pool_size, uri_length + 1);
  memcpy(object->pool, uri, uri_length);
  object->pool[uri_length] = '\0';

  uri_parse(object);
  if((mode.flag & URI_MODE_CANNONICAL) && uri_cannonicalize(object) != URI_CANNONICAL) {
    uri_free(object);
    return 0;
  } else {
    return object;
  }
}

int uri_realloc(uri_t* object, char* uri, int uri_length)
{
  uri_clear(object);

  static_alloc(&object->pool, &object->pool_size, uri_length + 1);
  memcpy(object->pool, uri, uri_length);
  object->pool[uri_length] = '\0';
  uri_parse(object);
  return uri_cannonicalize(object);
}

void uri_free(uri_t* object)
{
  uri_clear(object);
#define D(w) if(object->w) free(object->w)
  D(uri);
  D(furi);
  D(robots);
  D(pool);
#undef D
  if(object->cannonical) uri_free(object->cannonical);
  free(object);
}

/*
 * Object manipulation routines
 */

void uri_clear(uri_t* object)
{
#define D(w,f) \
  if(object->info & URI_INFO_M_##f) \
    free(object->w); \
  object->w = 0
  D(scheme,SCHEME);
  D(host,HOST);
  D(port,PORT);
  D(path,PATH);
  D(params,PARAMS);
  D(query,QUERY);
  D(frag,FRAG);
  D(user,USER);
  D(passwd,PASSWD);
#undef D
  object->info = 0;
}

uri_t* uri_clone(uri_t* from)
{
  uri_t* to = uri_alloc_1();
  uri_copy(to, from);
  return to;
}

/*
 * Copy an uri object into another. Cache information is not kept,
 * the cannonical form is not copied, the URL string is allocated in
 * pool, even if each field has its own malloc'd space in the source.
 * The destination object need not be a virgin object.
 */
void uri_copy(uri_t* to, uri_t* from)
{
  static_alloc(&to->pool, &to->pool_size, uri_estimate_pool_size(from));

  {
    int length;
    char* p = to->pool;
#define reloc(w,f) \
  if(to->info & URI_INFO_M_##f) \
    free(to->w); \
  if(from->w) { \
    length = strlen(from->w); \
    memcpy(p, from->w, length + 1); \
    to->w = p; \
    p += length + 1; \
  } else { \
    to->w = 0; \
  }
    reloc(scheme,SCHEME);
    reloc(host,HOST);
    reloc(port,PORT);
    reloc(path,PATH);
    reloc(params,PARAMS);
    reloc(query,QUERY);
    reloc(frag,FRAG);
    reloc(user,USER);
    reloc(passwd,PASSWD);
  }
#undef reloc
  to->info = from->info & ~URI_INFO_COPY_MASK;
#define D(w) \
  if(to->w) to->w[0] = '\0'
  D(uri);
  D(furi);
  D(robots);
#undef D
  to->desc = from->desc;
  to->cannonical = 0;
}

/*
 * Convert structure into string.
 */
char* uri_uri(uri_t* object)
{
  if((object->info & URI_INFO_URI) == 0) {
    uri_string(object, &object->uri, &object->uri_size, URI_STRING_URI_STYLE);
    object->info |= URI_INFO_URI;
  }
  return object->uri;
}

char* uri_furi(uri_t* object)
{
  if((object->info & URI_INFO_FURI) == 0) {
    uri_string(object, &object->furi, &object->furi_size, URI_STRING_FURI_STYLE);
    object->info |= URI_INFO_FURI;
  }
  return object->furi;
}

char* uri_furi_string(char* uri, int uri_length, int flag)
{
  static char* path = 0;
  static int path_size = 0;
  uri_t* object = uri_object(uri, uri_length);
  char* furi = uri_furi(object);
  int furi_length;
  char* wlroot = 0;

  if(!furi) return 0;

  furi_length = strlen(furi);

  if(flag & URI_FURI_REAL_PATH) {
    wlroot = getenv("WLROOT");
  }
  if(wlroot == 0)
    wlroot = "";
  
  static_alloc(&path, &path_size, strlen(wlroot) + furi_length + 32);
  
  sprintf(path, "%s%s%s.store", wlroot, (wlroot ? "/" : ""), furi);

  return path;
}

/*
 * Consistency checks
 */
int uri_consistent(uri_t* object)
{
  if(object->info & URI_INFO_FIELD_CHANGED) {
    char* uri = uri_uri(object);
    uri_realloc(object, uri, strlen(uri));
  }
  return object->info & URI_INFO_PARSED;
}

/*
 * URL cannonicalization generic machanism. Referch to the actual definition
 * of the 'specs' table in each uri_scheme_???.c file for a definition
 * of the allowed character set.
 */

static char hex2char[128] = {
/*  00 nul  01 soh   02 stx  03 etx   04 eot  05 enq   06 ack  07 bel   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  08 bs   09 ht    0a nl   0b vt    0c np   0d cr    0e so   0f si    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  10 dle  11 dc1   12 dc2  13 dc3   14 dc4  15 nak   16 syn  17 etb   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  18 can  19 em    1a sub  1b esc   1c fs   1d gs    1e rs   1f us    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  20 sp   21 !     22 "    23 #     24 $    25 %     26 &    27 '     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  28 (    29 )     2a *    2b +     2c ,    2d -     2e .    2f /     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  30 0    31 1     32 2    33 3     34 4    35 5     36 6    37 7     */
    0,      1,       2,      3,       4,      5,       6,      7,    
/*  38 8    39 9     3a :    3b ;     3c <    3d =     3e >    3f ?     */
    8,      9,       '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  40 @    41 A     42 B    43 C     44 D    45 E     46 F    47 G     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  48 H    49 I     4a J    4b K     4c L    4d M     4e N    4f O     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  50 P    51 Q     52 R    53 S     54 T    55 U     56 V    57 W     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  58 X    59 Y     5a Z    5b [     5c \    5d ]     5e ^    5f _     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  60 `    61 a     62 b    63 c     64 d    65 e     66 f    67 g     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  68 h    69 i     6a j    6b k     6c l    6d m     6e n    6f o     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  70 p    71 q     72 r    73 s     74 t    75 u     76 v    77 w     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  78 x    79 y     7a z    7b {     7c |    7d }     7e ~    7f del   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
};

static char char2hex[16] = "0123456789ABCDEF";

int cannonicalize_component(uri_t* object, char* from, char* to, int spec, char* spec_string)
{
  char* to_base = to;
  char* from_base = from;
  int* specs = object->desc->specs;

  int esc_spec = spec << 16;
  int mask = spec | esc_spec;

  while(*from) {
    unsigned char c;
    int coded;
    if(*from == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
      c = (hex2char[from[1] & 0x7f] << 4) | hex2char[from[2] & 0x7f];
      from += 3;
      coded = 1;
    } else {
      c = *from++;
      coded = 0;
    }
    
    if(specs[c] & mask) {
      /*
       * Code if 1) specs[c] is spec AND esc_spec and the
       *            character was already coded. (must be left untouched)
       *         2) specs[c] say it must be escaped.
       */
      int code = ((specs[c] & mask) == mask) ? coded : (specs[c] & esc_spec);
      if(code) {
	*to++ = '%';
	*to++ = char2hex[(c >> 4) & 0xf];
	*to++ = char2hex[c & 0xf];
      } else {
	*to++ = c;
      }
    } else {
      uri_error(0, "cannonicalize_component: illegal char %c in context %s\n", c, spec_string);
      return -1;
    }
  }
  *to = '\0';

  return to - to_base + 1;
}

char* uri_cannonicalize_string(char* uri, int uri_length, int flag)
{
  uri_t* object = uri_object(uri, uri_length);
  if(!object) return 0;
  if(!uri_cannonicalp(object) && uri_cannonicalize(object) != URI_CANNONICAL) return 0;
  if((mode.flag & URI_MODE_CANNONICAL) == 0)
    object = object->cannonical;
  uri_string(object, &object->uri, &object->uri_size, flag);
  return object->uri;
}

uri_t* uri_cannonical(uri_t* object)
{
  if(uri_cannonicalize(object) != URI_CANNONICAL)
    return 0;
  if(mode.flag & URI_MODE_CANNONICAL)
    return object;
  else
    return object->cannonical;
}

/*
 * In another file to prevent messing emacs with 8 bit chars
 */
#include "uri_escape_string.h"

/*
 * Escaping
 */
char* uri_escape(char* string, char* range)
{
  static char* escaped = 0;
  static int escaped_size = 0;
  int string_length = strlen(string);
  char* p;

  if(!range || strlen(range) == 0)
    range = uri_escape_default;

  static_alloc(&escaped, &escaped_size, string_length * 3);

  p = escaped;
  while(*string) {
    if(strchr(range, *string)) {
      unsigned char c = *string;
      *p++ = '%';
      *p++ = char2hex[(c >> 4) & 0xf];
      *p++ = char2hex[c & 0xf];
    } else {
      *p++ = *string;
    }
    string++;
  }
  *p = '\0';

  return escaped;
}

char* uri_unescape(char* string)
{
  static char* unescaped = 0;
  static int unescaped_size = 0;
  
  int string_length = strlen(string);

  char* p;
  static_alloc(&unescaped, &unescaped_size, string_length + 1);
  
  p = unescaped;
  while(*string) {
    unsigned char c;
    int coded;
    if(*string == '%' && isxdigit(string[1]) && isxdigit(string[2])) {
      *p = (hex2char[string[1] & 0x7f] << 4) | hex2char[string[2] & 0x7f];
      string += 3;
    } else {
      *p = *string++;
    }
    p++;
  }
  *p = '\0';

  return unescaped;
}

/*
 * Dump uri_t content
 */
void uri_dump(uri_t* object)
{
  printf("flags: ");
#define flag(w) if(object->info & w) printf(#w " ")
  flag(URI_INFO_CANNONICAL);
  flag(URI_INFO_URI);
  flag(URI_INFO_FURI);
  flag(URI_INFO_ROBOTS);
  flag(URI_INFO_RELATIVE);
  flag(URI_INFO_RELATIVE_PATH);
  flag(URI_INFO_EMPTY);
  flag(URI_INFO_PARSED);
  flag(URI_INFO_M_SCHEME);
  flag(URI_INFO_M_HOST);
  flag(URI_INFO_M_PORT);
  flag(URI_INFO_M_PATH);
  flag(URI_INFO_M_PARAMS);
  flag(URI_INFO_M_QUERY);
  flag(URI_INFO_M_FRAG);
  flag(URI_INFO_M_USER);
  flag(URI_INFO_M_PASSWD);
#undef flag
  printf("\n");

  if(object->info & URI_INFO_CANNONICAL) printf("cannonical form\n");
  if(object->scheme) printf("scheme: %s\n", object->scheme);
  if(object->host) printf("host: %s\n", object->host);
  if(object->port) printf("port: %s\n", object->port);
  if(object->path) printf("path: %s\n", object->path);
  if(object->params) printf("params: %s\n", object->params);
  if(object->query) printf("query: %s\n", object->query);
  if(object->frag) printf("frag: %s\n", object->frag);
  if(object->user) printf("user: %s\n", object->user);
  if(object->passwd) printf("passwd: %s\n", object->passwd);
}

