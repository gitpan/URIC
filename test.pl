BEGIN {print "1..5\n";}
END {print "not ok 1\n" unless $loaded;}
use uri;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

uri_mode($URI_MODE_CANNONICAL);

$uri = "http://www.dummy.fr/foo?bar";
$uri_object = uri_alloc($uri, length($uri));

$uri eq uri_uri($uri_object) ? print "ok 2\n" : print "not ok 2\n";

$uri_object->{'path'} eq 'foo' ? print "ok 3\n" : print "not ok 3 \n";

uri_free($uri_object);
print "ok 4\n";

$uri = "http://www,dummy!fr/";
$uri_object = uri_alloc($uri, length($uri));
!defined($uri_object) ? print "ok 5\n" : print "not ok 5 \n";
