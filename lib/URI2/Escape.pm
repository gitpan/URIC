package URI2::Escape;
use strict;

use URIC;

use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION);
use vars qw(%escapes);

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(uri_escape uri_unescape);
@EXPORT_OK = qw(%escapes);

use Carp ();

sub uri_escape
{
    my($text, $patn) = @_;
    $patn = '' if(!defined($patn));
    return URICc::uri_escape($text, $patn);
}

sub uri_unescape
{
    return URICc::uri_unescape(@_);
}
