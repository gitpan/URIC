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

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#include <getopt.h>
#include <uri.h>

static void parse_stdin(int dump);
static void run_test();
static void run_speed();
static void time_register(time_t* timevar);
static void time_show(char* message, time_t timevar);

main(argc, argv)
int argc;
char** argv;
{
  static int stdin_p = 0;
  static int run_test_p = 0;
  static int stdin_dump_p = 0;
  static int verbose = 0;
  int run_speed_loop = 0;
  
  while(1) {
    static struct option long_options[] =
      {
	/* These options set a flag. */
	{"verbose", 0, &verbose, 1},
	{"no_cannonical", 0, 0, 0},
	{"uri_strict", 0, 0, 0},
	{"field_malloc", 0, 0, 0},
	{"run_test", 0, &run_test_p, 1},
	{"run_speed", 1, 0, 0},
	{"stdin", 0, &stdin_p, 1},
	{"stdin_dump", 0, &stdin_dump_p, 1},
	{0, 0, 0, 0}
      };
    /* `getopt_long' stores the option index here. */
    int option_index = 0;
    int c;

    c = getopt_long_only(argc, argv, "", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;
     
    switch (c)
      {
      case 0:
	/* If this option set a flag, do nothing else now. */
	if (long_options[option_index].flag != 0)
	  break;
	if(!strcmp(long_options[option_index].name, "no_cannonical")) {
	  uri_mode_set(uri_mode() & ~URI_MODE_CANNONICAL);
	} else if(!strcmp(long_options[option_index].name, "field_malloc")) {
	  uri_mode_set(uri_mode() | URI_MODE_FIELD_MALLOC);
	} else if(!strcmp(long_options[option_index].name, "uri_strict")) {
	  uri_mode_set(uri_mode() | URI_MODE_URI_STRICT);
	} else if(!strcmp(long_options[option_index].name, "run_speed")) {
	  run_speed_loop = atoi(optarg);
	}
	break;
      default:
	fprintf(stderr, "option parse error %c, 0x%x\n", c & 0xff, c);
	exit(1);
      }
  }

  if(stdin_p)
    parse_stdin(stdin_dump_p);
  if(run_test_p)
    run_test();
  if(run_speed_loop > 0)
    run_speed(run_speed_loop, &argv[optind]);
}

#define DUMMY "http://www.dumy.fr/"

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

/*
 * Sample run
 *
 * bash$ ./t_uri < t_uri.input > 1
 * uri_cannonicalize_1: illegal char / in context SPEC_NETLOC
 * bash$ ./t_uri < 1 > 2
 * bash$ ./t_uri < 2 > 3
 * bash$ diff 2 3
 * bash$ 
 *
 * t_uri2.input contains tests about convertion to absolute
 * URL from relative URLs and base.
 */
static void parse_stdin(int dump)
{
  uri_t* uri = uri_alloc_1();
  uri_t* base_uri = uri_alloc_1();

  while(fgets(buffer, BUFFER_SIZE, stdin)) {
    int length;
    if((buffer[0] == '#' && buffer[1] == '\n') ||
       (buffer[0] == '#' && buffer[1] == ' ')) {
      fprintf(stderr, "%s", buffer);
      continue;
    }
    /* - 1 to suppress the trailing newline. */
    fprintf(stderr, "#\n# %s#\n", buffer);
    buffer[strlen(buffer) - 1] = '\0';
    {
      char* base;
      base = strchr(buffer, ' ');
      if(base) {
	*base++ = '\0';
      }
      if(uri_realloc(uri, buffer, strlen(buffer)) == URI_CANNONICAL) {
	fprintf(stderr, "uri: %s\n", uri_uri(uri));
	fprintf(stderr, "furi: %s\n", uri_furi(uri));
	if(dump)
	  uri_dump(uri);
	if(base && (uri->info & URI_INFO_RELATIVE)) {
	  if(uri_realloc(base_uri, base, strlen(base)) == URI_CANNONICAL) {
	    uri_t* absolute = uri_abs_1(base_uri, uri);
	    fprintf(stderr, "uri base: %s\n", uri_uri(base_uri));
	    if(absolute) {
	      fprintf(stderr, "uri absolute: %s\n", uri_uri(absolute));
	      if(dump)
		uri_dump(absolute);
	    }
	  }
	}
      }
    }
  }
  uri_free(uri);
  uri_free(base_uri);
}

static void run_test()
{
  uri_t* uri = uri_alloc_1();
  {
    char* urls[] = {
      "http://www.ceic.com/",
      "http://www.cei%43.com/",
      "http://www.ceic.com/a/b/",
      "http://www.ceic.com/a%2f/a&/",
      "http://www.ceic.com/a?foo=1&bar=2",
      "http://www.ceic.com/a?foo=1&bar=%26&foobar=3",
      0
    };
    char* url;
    int i;
  
    for(i = 0; urls[i]; i++) {
      printf("uri = %s\n", urls[i]);
      uri_realloc(uri, urls[i], strlen(urls[i]));
      printf("\thost = %s\n", uri_host(uri));
      printf("\tpath = %s\n", uri_path(uri));
      printf("\tquery = %s\n", uri_query(uri));
    }
  }

  {
    char* url = "http://www.ceic.com/foo%7e/a%/b";
    printf("uri_unescape path %s = %s\n", url, uri_unescape(url));
    printf("uri_escape path %s = %s\n", url, uri_escape(uri_unescape(url), "%"));
  }

  {
    char* url = "file:/a/b/c.html";
    uri_realloc(uri, url, strlen(url));
    uri_scheme_set(uri, "http");
    printf("uri scheme change (file -> http) %s \n", uri_uri(uri));
  }

  {
    char* url = "file://web/orig/path";
    uri_realloc(uri, url, strlen(url));
    uri_scheme_set(uri, "http");
    uri_path_set(uri, "1info");
    uri_query_set(uri, "key words");
    uri_frag_set(uri, "this");
    uri_cannonicalize(uri);
    printf("uri change %s -> %s \n", url, uri_uri(uri));
  }

  {
    char* url = "gopher://gopher:33/3S%09query";
    uri_realloc(uri, url, strlen(url));
    uri_path_set(uri, "45%09a");
    uri_cannonicalize(uri);
    printf("gopher_type  %s -> %s \n", uri_uri(uri), uri_gopher_type(uri));
  }
}

static void run_speed(int loop, char** urls)
{
  int i;
  uri_t* uri = uri_alloc_1();
  char** p;
  int url_count = 0;

  for(p = urls; *p; p++) 
    url_count++;

  time_register(0);
  
  for(i = 0; i < loop; i++) {
    int j;
    for(j = 0; j < url_count; j++)
      uri_realloc(uri, urls[j], strlen(urls[j]));
  }

  {
    char message[512];
    sprintf(message, "loop %d over %d urls = %d iterations of url_realloc ", loop, url_count, loop * url_count);
    time_show(message, 0);
  }
}

static time_t time_base;

static void time_register(time_t* timevar)
{
  if(timevar) {
    *timevar = time(0);
  } else {
    time_base = time(0);
  }
}

static void time_show(char* message, time_t timevar)
{
  if(timevar == 0)
    timevar = time(0) - time_base;
  fprintf(stderr, "%s (%dh:%dm:%ds) \n", message, (timevar / 3600), ((timevar % 3600) / 60), (timevar % 60));
}
