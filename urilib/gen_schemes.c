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

#include "uri_schemes_hash.c"

static char* scheme_map[MAX_HASH_VALUE];

main(int argc, char** argv)
{
  int i;
  FILE* fp;

  for(i = 1; i < argc; i++) {
    int index = uri_scheme_hash(argv[i], strlen(argv[i]));
    if(!strcmp("generic", argv[i]))
      continue;
    if(uri_scheme_exists(argv[i], strlen(argv[i])) == 0) {
      fprintf(stderr, "%s: unexpected scheme %s, skipped\n", argv[i]);
      continue;
    }
    scheme_map[index] = argv[i];
  }

  /*
   * Build description branch table
   */
  fp = fopen("uri_schemes_desc.c", "w");
  if(!fp) {
    perror("could not open uri_scheme_desc.c for writing");
    exit(1);
  }
  for(i = 0; i < MAX_HASH_VALUE; i++) {
    if(uri_scheme_list[i][0] == '\0') {
      fprintf(fp, "/* %02d */ \t0,\n", i);
    } else {
      char* scheme = scheme_map[i] ? scheme_map[i] : "generic";
      fprintf(fp, "/* %02d */ \t&uri_scheme_%s_desc,\t/* %s */\n", i, scheme, uri_scheme_list[i]);
    }
  }
  fclose(fp);

  /*
   * Build description structures declarations
   */
  fp = fopen("uri_schemes_desc.h", "w");
  if(!fp) {
    perror("could not open uri_scheme_desc.h for writing");
    exit(1);
  }
  for(i = 0; i < MAX_HASH_VALUE; i++) {
    if(scheme_map[i] != 0) {
      fprintf(fp, "/* %02d */ extern uri_scheme_desc_t uri_scheme_%s_desc; /* %s */\n", i, scheme_map[i], uri_scheme_list[i]);
    }
  }
  fclose(fp);

  /*
   * Build objects list makefile
   */
  fp = fopen("uri_schemes.mk", "w");
  if(!fp) {
    perror("could not open uri_scheme.mk for writing");
    exit(1);
  }
  fprintf(fp, "URI_SCHEMES = ");
  for(i = 0; i < MAX_HASH_VALUE; i++) {
    if(scheme_map[i] != 0) {
      fprintf(fp, "liburi.a(uri_scheme_%s.o) ", scheme_map[i]);
    }
  }
  fprintf(fp, "\n");
  fclose(fp);

  exit(0);
}
