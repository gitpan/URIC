#!/local/bin/perl -w

use URIC;

print "1..4\n";

uri_mode_set($URI_MODE_CANNONICAL);

$uri = "http://www.dummy.fr/foo?bar";
$uri_object = uri_alloc($uri, length($uri));

$uri eq uri_uri($uri_object) ? print "ok 1\n" : print "not ok 1\n";

uri_path($uri_object) eq 'foo' ? print "ok 2\n" : print "not ok 2 \n";

uri_free($uri_object);
print "ok 3\n";

$uri = "http://www,dummy!fr/";
$uri_object = uri_alloc($uri, length($uri));
!defined($uri_object) ? print "ok 4\n" : print "not ok 4 \n";
