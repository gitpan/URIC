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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>

#include "uri.h"

#define DUMMY "http://www.dumy.fr/"

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

main(argc, argv)
int argc;
char** argv;
{
  uri_t* uri = uri_alloc(DUMMY, strlen(DUMMY));
  uri_t* base_uri = uri_alloc(DUMMY, strlen(DUMMY));

  while(fgets(buffer, BUFFER_SIZE, stdin)) {
    int length;
    if((buffer[0] == '#' && buffer[1] == '\n') ||
       (buffer[0] == '#' && buffer[1] == ' '))
      continue;
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
	if(argv[1])
	  uri_dump(uri);
	if(base && (uri->info & URI_INFO_RELATIVE)) {
	  if(uri_realloc(base_uri, base, strlen(base)) == URI_CANNONICAL) {
	    uri_t* absolute = uri_abs_1(base_uri, uri);
	    fprintf(stderr, "uri base: %s\n", uri_uri(base_uri));
	    if(absolute) {
	      fprintf(stderr, "uri absolute: %s\n", uri_uri(absolute));
	      if(argv[1])
		uri_dump(absolute);
	    }
	  }
	}
      }
    }
  }
  uri_free(uri);
/*  printf("%s\n", uri_furi(uri));*/
}
