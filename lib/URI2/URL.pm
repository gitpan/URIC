#
# Incompatibilities (+ after imcompatibilities that should probably be
# fixed) :
#
# . \0 is not permitted in strings, %00 is permitted.              +
# . in netloc the following is not possible because not
#   distinction is done between null string and undefined string.
# 
#   :0@h:0      missing user, auth part omitted (h:0)
#   :@h:0       missing user, auth part omitted (h:0)
#   foo:@h:0    missing passwd, passwd omitted (foo@h:0)           +
#
# . crack not implemented                                          +
# . derivation not implemented                                     +
# . leading . and .. are reduced
#      /./g       = <URL:http://a/./g>                             +
#      /../g      = <URL:http://a/../g>                            +
#
# . a/b/c//../d -> a/b/d instead of a/b/c/d. This is how
#   filesystems behave.
# . gopher: is an error. In the old implementation it was valid. 
# . file:a is the same as file:/a. When using Netscape, file:a has
#   no defined behaviour anyway.
# . In file://localhost/a/b/c localhost is implicit and resulting
#   string (as_string) will return file:/a/b/c
# . rel function will return "" instead of "./" for 
#   rel(http://a/b/c, http://a/b/c)
# 
# Escaping
#
# The escape char specification for uri_escape() may not contain
# intervals like \x00-\x20 but should contain all the characters
# explicitly.
#
# . $reserved 
# . $reserved_no_slash
# . $reserved_no_form
# . $unsafe
# . implicit
#   ($unsafe + some $reserved chars, not clear why not all reserved chars
#    were set)
#
# Per component escaping policy
#
# . path($value), params($value), query($value) escape $value before set
#   + return old value unescaped and check that no information is lost
#   when unescaping. No check is don't on $value, if it contains % the
#   % are escaped. $value must be unescaped. Specific chars are
#   escaped, not all : reserved_no_slash for path, reserved_no_form for query
#   and params.
#   Drawback: control chars are not escaped, no control on % loss when setting,
#   deep understanding necessary to get the expected result : must explicitly
#   call uri_escpape for accented letters and control chars. The intuitive
#   $object->path("a/b/c^ad/"); $object->epath() will give the original
#   string instead of the expected escaped "a/b/c%01d/".
#
# . epath($value), eparams($value), equery($value) store + retrieve verbatim
#   the name suggest that the values stored and retrieved must be escaped
#   and unescaped by the caller.
#   Drawback:
#
# . full_path
#   return path + param + query with 'unsafe' chars escaped
#   with aditional argument return path + param + query as is
#   Drawback: inconsistent with path(), params() and query() which return
#   the unescaped value. Escaping when setting path() params() and query() 
#   use 'reserved' chars instead of 'unsafe'. One would expect full_path
#   to be strictly equivalent to path() . ";" . params() . "?" . query()
#   which is not the case. Escaping is not done on a per component basis
#   and reserved chars are not escaped.
#
# . as_string is similar to full_path
#
# . path_component(@list) store in 'path' the path built from the 
#   @list, reserved chars quoted + '.'. Unescape each path component
#   of the existing path a list of them. This is approximately consistent 
#   with path().
#   Drawback: inconsistent with path() regarding the handling of . as
#   a reserved char. See also full path.
# 
# Absolute and relative path
#   
#   The situation is quite confuse regarding the leading / of the path.
#   Many functions contained special cases to restore the leading / when
#   'necessary'. This has been changed by introducing two functions that
#   handle path names given by the caller (path_in) and path names 
#   returned to the caller. A flag in the object specifies if the path
#   is absolute or relative. This is necessary because a path not beginning
#   with a / is absolute in an absolute URL and relative in a relative URL.
#   Therefore the path only does not allow to find out if it's absolute or
#   relative. 
#
# . Nulls in strings are not allowed, C library do not like them
#
# . If user is set to '' or passwd set to '' they do not show
#   See
#    $url->user('');
#    $url->_expect('netloc' => ':0@h:0');
#    $url->password('');
#    $url->_expect('netloc' => ':@h:0');
#    $url->user('foo');
#    $url->_expect('netloc' => 'foo:@h:0');
#   in t/base.t
#  
# . news : path is not overriden (no object orientation)
#
package URI2::URL;

$VERSION = "5.00";   

sub Version { $VERSION; }

require 5.004;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(url);

use Carp ();
use URIC;

#use strict 'vars';
#use strict 'subs';

use vars qw(
	    $Debug $Strict_URL $reserved $reserved_no_slash $reserved_no_form
	   );

$reserved          = ";\\/?:\\@&=+#%"; # fielding-03 reserved plus '#' and '%'
$reserved_no_slash = ";?:\\@&=+#%";    # used when escaping path
$reserved_no_form  = ";\\/?:\\@#%";    # used when escaping params and query

$Debug         = 0;     # set to 1 to print URLs on creation
$Strict_URL    = 0;     # see new()

use overload ( '""' => 'as_string', 'fallback' => 1 );

URICc::uri_mode_set($URICc::URI_MODE_LOWER_SCHEME | $URICc::URI_MODE_FIELD_MALLOC | $URICc::URI_MODE_URI_STRICT_SCHEME);

# Easy to use constructor
sub url ($;$)
{
    URI2::URL->new(@_);
}

sub new
{
    my($class, $init, $base) = @_;

    my $self;
    if (ref $init) {
	$self = $init->clone;
    } else {
	$init = "" unless defined $init;
	# RFC 1738 appendix suggest that we just ignore extra whitespace
	$init =~ s/\s+//g;
	# Also get rid of any <URL: > wrapper
	$init =~ s/^<(?:URL:)?(.*)>$/$1/;

	# hand-off to scheme specific implementation sub-class
	$self->{'_orig_url'} = $init if $Debug;
	$self->{'uri'} = URICc::uri_alloc($init, length($init));
	if(!defined($self->{'uri'})) {
	    Carp::croak("URI2::URL $URICc::uri_errstr")
	}
    }

    bless($self, $class);
    if($base) {
	if(!ref($base)) {
	    $base = URI2::URL->new($base);
	}
	#
	# Convert base to absolute form.
	#
	$base = URI2::URL->new($base->abs());
	$self->base($base);
	my($scheme) = URICc::uri_scheme($base->{'uri'});
	if($scheme && !URICc::uri_scheme($self->{'uri'})) {
	    $init = "$scheme:$init";
	    if(URICc::uri_realloc($self->{'uri'}, $init, length($init)) != $URI_CANNONICAL) {
		Carp::croak("URI2::URL $URICc::uri_errstr");
	    }
	    URICc::uri_scheme_set($self->{'uri'}, '');
	}
    }
    $self->print_on('STDERR') if $Debug;
    return $self;
}

sub clone
{
    my $self = shift;
    # this work as long as none of the components are references themselves
    return URI2::URL->new($self->as_string());
}

# This private method help us implement access to the elements in the
# URI2::URL object hash (%$self).  You can set up access to an element
# with a routine similar to this one:
#
#  sub component { shift->_elem('component', @_); }

%URL::elemc = (
	       'scheme' => 1,
	       'host' => 1,
	       'port' => 1,
	       'path' => 1,
	       'params' => 1,
	       'query' => 1,
	       'frag' => 1,
	       'user' => 1,
	       'passwd' => 1,
	       );
sub _elem
{
    my $self = shift;
    my $elem = shift;
    my($old);
    if($URL::elemc{$elem}) {
	my($access) = "URICc::uri_${elem}";
	$old = &$access($self->{'uri'});
	if (@_) {
	    my $set = "URICc::uri_${elem}_set";
	    my($elem) = shift;
	    $elem = '' if(!defined($elem));
	    &$set($self->{'uri'}, $elem);
	}
	undef($old) if(!length($old));
    } else {
	$old = $self->{$elem};
	if (@_) {
	    $self->{$elem} = shift;
	}
    }
    $old;
}

sub DESTROY {
    my($self) = @_;

    if(defined($self->{'uri'}))  {
      URICc::uri_free($self->{'uri'});
	undef($self->{'uri'});
    }
}

sub newlocal
{
    my($class, $file) = shift;
    return URI2::URL->new("file:$file");
}

sub strict
{
    return $Strict_URL unless @_;
    my $old = $Strict_URL;
    $Strict_URL = $_[0];
    $old;
}

sub base { shift->_elem('base', @_); }
sub scheme { shift->_elem('scheme', @_); }

sub crack
{
    my $self = shift;
    my($o) = $self->{'uri'};
    (URICc::uri_scheme($o),  	  # 0: scheme
     URICc::uri_user($o),          # 1: user
     URICc::uri_passwd($o),        # 2: passwd
     URICc::uri_host($o),          # 3: host
     URICc::uri_port($o),          # 4: port
     URICc::uri_path($o),          # 5: path
     URICc::uri_params($o),        # 6: params
     URICc::uri_query($o),         # 7: query
     URICc::uri_frag($o)           # 8: fragment
    )
}

sub abs
{
    my($self, $base, $allow_scheme_in_relative_urls) = @_;

    if($base) {
	$base = new URI2::URL($base);
    } else {
	$base = $self->base();
    }

    return $self->clone unless $base;            # we have no base (step1)

    URICc::uri_mode_set(URICc::uri_mode() & ~$URI_MODE_URI_STRICT_SCHEME) if($allow_scheme_in_relative_urls);

    my($absolute) = URICc::uri_uri(URICc::uri_abs_1($base->{'uri'}, $self->{'uri'}));
    my($ret) = URI2::URL->new($absolute);

    URICc::uri_mode_set(URICc::uri_mode() | $URI_MODE_URI_STRICT_SCHEME) if($allow_scheme_in_relative_urls);

    return $ret;
}

sub as_string { return URICc::uri_escape(URICc::uri_uri(shift->{'uri'}), $uri_escape_unsafe); }

# This is set up as an alias for various methods
sub bad_method {
    my $self = shift;
    my $scheme = $self->scheme;
    Carp::croak("Illegal method called for $scheme: URL")
	if $Strict_URL;
    # Carp::carp("Illegal method called for $scheme: URL")
    #     if $^W;
    undef;
}

sub print_on
{
    my($self) = @_;

    URICc::uri_dump($self->{'uri'});
}

sub epath {
    my($self) = shift;
    my($path) = @_;
    my($pathp);
    $pathp = 1 if(@_);

    #
    # Get the old path before changing to new because we may change the
    # relative_path flag.
    #
    my($path_out) = $self->path_out();
    if($pathp) {
	my($uri) = $self->{'uri'};
	#
	# Compute the path string and find out if it's a relative
	# or absolute path
	#
	my($path_in, $relative) = $self->path_in($path);
	#
	# Set the new path and relative_path flag if appropriate;
	#
	$self->_elem('path', $path_in);
        URICc::uri_info_set($uri, URICc::uri_info($uri) | $URI_INFO_RELATIVE_PATH) if($relative);
    }
    return $path_out;
}

sub path {
    my($self) = shift;

    my($path_out);
    if(@_) {
	my($path) = URICc::uri_escape($_[0], $reserved_no_slash);
	$path_out = $self->epath($path);
    } else {
	$path_out = $self->epath();
    }
    #
    # Sanity checks
    #
    Carp::croak("Path components contain '/' (you must call epath)")
	if $path_out =~ /%2[fF]/ and !@_;

    return URICc::uri_unescape($path_out);
}

sub params {
    my $self = shift;
    my $old = $self->_elem('params', map {URICc::uri_escape($_,$reserved_no_form)} @_);
    return URICc::uri_unescape($old) if defined $old;
    undef;
}

sub query {
    my $self = shift;
    my $old = $self->_elem('query', map { URICc::uri_escape($_, $reserved_no_form) } @_);
    if (defined(wantarray) && defined($old)) {
	if ($old =~ /%(?:26|2[bB]|3[dD])/) {  # contains escaped '=' '&' or '+'
	    my $mess;
	    for ($old) {
		$mess = "Query contains both '+' and '%2B'"
		  if /\+/ && /%2[bB]/;
		$mess = "Form query contains escaped '=' or '&'"
		  if /=/  && /%(?:3[dD]|26)/;
	    }
	    if ($mess) {
		Carp::croak("$mess (you must call equery)");
	    }
	}
	# Now it should be safe to unescape the string without loosing
	# information
	return URICc::uri_unescape($old);
    }
    undef;
}

sub host {
    my($self) = shift;
    my($tmp) = $self->_elem('host', map { URICc::uri_escape($_, $reserved) } @_);
    return defined($tmp) ? URICc::uri_unescape($tmp) : undef;
}

sub port {
    my($self) = shift;
    my($tmp) = $self->_elem('port', map { URICc::uri_escape($_, $reserved) } @_);
    return defined($tmp) ? URICc::uri_unescape($tmp) : undef;
}

sub eparams { shift->_elem('params', @_); }
sub equery { shift->_elem('query', @_); }
sub frag { shift->_elem('frag', @_); }

sub user {
    my($self) = shift;
    my($tmp) = $self->_elem('user', map { URICc::uri_escape($_, $reserved) } @_);
    return defined($tmp) ? URICc::uri_unescape($tmp) : undef;
}

sub password {
    my($self) = shift;
    my($tmp) = $self->_elem('passwd', map { URICc::uri_escape($_, $reserved) } @_);
    return defined($tmp) ? URICc::uri_unescape($tmp) : undef;
}

sub netloc {
    return URICc::uri_auth_netloc(shift->{'uri'});
}

sub auth {
    my($auth) = URICc::uri_auth(shift->{'uri'});
    return length($auth) > 0 ? $auth : undef;
}

sub full_path {
  URICc::uri_escape(URICc::uri_all_path(shift->{'uri'}), $uri_escape_unsafe);
}

sub default_port {
  URICc::uri_default_port(shift->{'uri'});
}

sub path_components {
    my $self = shift;

    my($path_out);
    if(@_) {
	my($path) = join("/", map { URICc::uri_escape($_, $reserved.".") } @_);
	$path_out = $self->epath($path);
    } else {
	$path_out = $self->epath();
    }

    map { URICc::uri_unescape($_) } split("/", $path_out, -1);
}

#
# The path given may be
# . absolute path if url is absolute. If the path begins with a / 
#   or not, it is absolute anyway since a relative path means nothing
#   in an absolute url
# . relative path if url is relative AND path does not start with a / 
# . absolute path if url is relative AND path starts with a / 
#
# Returns ($path, $relative) where $path is the string that may
# be inserted in the path component and $relative is set if it's a
# relative path, unset if not.
#
sub path_in {
    my($self, $path) = @_;
    my($uri) = $self->{'uri'};
    my($relative);

    return if(!defined($path));

    if(URICc::uri_info($uri) & $URI_INFO_RELATIVE) {
	$relative = 1 if($path !~ m|^/|);
    }
    $path =~ s|^/||;

    return ($path, $relative);
}

#
# Returns the path component with
# . Leading / added if the path is absolute, that is if the url is
#   absolute or if the url is relative and the path is absolute.
# . No leading / added if the url is relative and the path is relative.
#
sub path_out {
    my($self) = @_;
    my($path) = $self->_elem('path');
    my($uri) = $self->{'uri'};

    if((URICc::uri_info($uri) & $URI_INFO_RELATIVE) == 0 ||
       ((URICc::uri_info($uri) & $URI_INFO_RELATIVE) &&
	(URICc::uri_info($uri) & $URI_INFO_RELATIVE_PATH) == 0)) {
	if(!defined($path)) {
	    $path = "/";
	} else {
	    $path = "/$path";
	}
    }
    return $path;
}

sub eq {
    my($self, $other) = @_;

    $other = URI2::URL->new($other) if(!ref($other));

    my($a) = URICc::uri_cannonical($self->{'uri'});
    if(!defined($a)) {
      Carp::croak($self->as_string() . " cannot be cannonicalized");
    }
    my($b) = URICc::uri_cannonical($other->{'uri'});
    if(!defined($b)) {
      Carp::croak($other->as_string() . " cannot be cannonicalized");
    }

    return URICc::uri_escape(URICc::uri_uri($a), $uri_escape_unsafe) eq URICc::uri_escape(URICc::uri_uri($b), $uri_escape_unsafe);
}

sub rel {
    my($self, $base) = @_;
    my($rel) = $self->clone;
    $base = $self->base unless $base;
    return $rel unless $base;
    $base = new URI2::URL $base unless ref $base;

    $rel->base($base);
    my($scheme) = $rel->scheme();
    my($netloc) = $rel->netloc();
    if (!defined($scheme) && !defined($netloc)) {
	# it is already relative
	return $rel;
    }
    my($bscheme) = $base->scheme();
    my($bnetloc) = $base->netloc();

    for ($bscheme, $bnetloc, $netloc) { $_ = '' unless defined }

    unless ($scheme eq $bscheme && $netloc eq $bnetloc) {
	# different location, can't make it relative
	return $rel;
    }

    my($path) = $rel->path();
    my($bpath) = $base->path();

    for ($path, $bpath) {  $_ = "/$_" unless m,^/,; }

    # Make it relative by eliminating scheme and netloc
    $rel->scheme(undef);
    $rel->host(undef);
    $rel->port(undef);
    $rel->user(undef);
    $rel->password(undef);

    # This loop is based on code from Nicolai Langfeldt <janl@ifi.uio.no>.
    # First we calculate common initial path components length ($li).
    my $li = 1;
    while (1) {
	my $i = index($path, '/', $li);
	last if $i < 0 ||
                $i != index($bpath, '/', $li) ||
	        substr($path,$li,$i-$li) ne substr($bpath,$li,$i-$li);
	$li=$i+1;
    }
    # then we nuke it from both paths
    substr($path, 0,$li) = '';
    substr($bpath,0,$li) = '';

    if ($path eq $bpath && defined($rel->frag) && !defined($rel->equery)) {
        $rel->epath('');
    } else {
        # Add one "../" for each path component left in the base path
        $path = ('../' x $bpath =~ tr|/|/|) . $path;
#	$path = "./" if $path eq "";
        $rel->epath($path);
    }

    $rel->print_on('STDERR');
    $rel;
}

#
# Http specific
#
sub keywords {
    my($self) = shift;
    my($old) = $self->equery();
    if (@_) {
	# Try to set query string
	$self->equery(join('+', map { URICc::uri_escape($_, $reserved) } @_));
    }
    return if !defined($old) || !defined(wantarray);

    Carp::croak("Query is not keywords") if $old =~ /=/;
    map { URICc::uri_unescape($_) } split(/\+/, $old);
}

# Handle ...?foo=bar&bar=foo type of query
sub query_form {
    my $self = shift;
    my($old) = $self->equery();
    if (@_) {
	# Try to set query string
	my @query;
	my($key,$vals);
	while (($key,$vals) = splice(@_, 0, 2)) {
	    $key = '' unless defined $key;
	    $key =  URICc::uri_escape($key, $reserved);
	    $key =  URICc::uri_escape($key, $uri_escape_unsafe);
	    $vals = [$vals] unless ref($vals) eq 'ARRAY';
	    my $val;
	    for $val (@$vals) {
		$val = '' unless defined $val;
		$val = URICc::uri_escape($val, $reserved);
		$val = URICc::uri_escape($val, $uri_escape_unsafe);
		push(@query, "$key=$val");
	    }
	}
	$self->equery(join('&', @query));
    }
    return if !defined($old) || length($old) == 0 || !defined(wantarray);
    Carp::croak("Query is not a form") unless $old =~ /=/;
    map { s/\+/ /g; URICc::uri_unescape($_) }
	 map { /=/ ? split(/=/, $_, 2) : ($_ => '')} split(/&/, $old);
}

#
# Gopher specific
#
sub gtype { shift->_elem('user', @_); }
sub selector { shift->_elem('params', @_); }
sub search { shift->_elem('query', @_); }
sub string { shift->_elem('frag', @_); }

#
# Mailto specific
# 
sub address {
    my($self, $value) = @_;

    my($old) = URICc::uri_user($self->{'uri'}) . "@" . URICc::uri_host($self->{'uri'});
    if($value) {
	my($uri) = "mailto:$value";
        URICc::uri_realloc($self->{'uri'}, $uri, length($uri));
    }
    return $old;
}

*encoded822addr = \&address;

#
# News and nntp specific
#
sub article {
    my($self) = shift;
    my($func) = $self->scheme() . "_article";
    return &$func($self, @_);
}

sub group {
    my($self) = shift;
    my($func) = $self->scheme() . "_group";
    return &$func($self, @_);
}

#
# News specific
#
sub groupart {
    my $self = shift;
    my $old = $self->_elem('path');
    if (@_) {
	my $p = shift;
	if (defined $p && $p =~ /\@/) {
	    # it is an message id
	    $p =~ s/^<(.*)>$/$1/;  # "<" and ">" should not be part of it
	}
	$self->_elem('path', $p);
    }
    return $old;
}

sub news_article   {
    my $self = shift;
    Carp::croak("Illegal article id name (does not contain '\@')")
      if @_ && $_[0] !~ /\@/;
    my $old = $self->groupart(@_);
    return undef if $old !~ /\@/;
    $old;
}

sub news_group {
    my $self = shift;
    Carp::croak("Illegal group name (contains '\@')")
      if @_ && $_[0] =~ /\@/;
    my $old = $self->groupart(@_);
    return undef if $old =~ /\@/;
    $old;
}

#
# Nntp specific
#
sub nntp_group { shift->_elem('path', @_); }
sub nntp_article { shift->_elem('params', @_); }
sub digits { shift->_elem('params', @_); }

#
# Wais specific
#
sub database { shift->_elem('path', @_); }
sub wtype { shift->_elem('params', @_); }
sub wpath { shift->_elem('frag', @_); }

