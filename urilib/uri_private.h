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
#ifndef _uri_private_h
#define _uri_private_h

/*
 * Maximum number of chars expected as component separators
 */
#define URI_SEPARATORS	32

/*
 * Global parameters
 */
typedef struct mode {
  int flag;
} uri_mode_t;

extern uri_mode_t mode;

#define uri_estimate_pool_size(object) \
      (URI_SEPARATORS + \
       (object->scheme ? strlen(object->scheme) : 0) + \
       (object->host ? strlen(object->host) : 0) + \
       (object->port ? strlen(object->port) : 0) + \
       (object->path ? strlen(object->path) : 0) + \
       (object->params ? strlen(object->params) : 0) + \
       (object->query ? strlen(object->query) : 0) + \
       (object->frag ? strlen(object->frag) : 0) + \
       (object->user ? strlen(object->user) : 0) + \
       (object->passwd ? strlen(object->passwd) : 0))

#endif /* _uri_private_h */
