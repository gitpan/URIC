/* C code produced by gperf version 2.7 */
/* Command-line: gperf -k2-4 -c -G -W uri_scheme_list -H uri_scheme_hash -N uri_scheme_exists  */

#define TOTAL_KEYWORDS 39
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 101
/* maximum key range = 100, duplicates = 0 */

#ifdef __GNUC__
__inline
#endif
static unsigned int
uri_scheme_hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned char asso_values[] =
    {
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102,  10,  45,
       10,   0,  25, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102,  41,   5,  25,
       40,   0, 102,  15,   0,   0, 102, 102,   5,  45,
       35,  55,   0,  10,   0,   0,  15,  40,   0,  25,
        5, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
      102, 102, 102, 102, 102, 102
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
        break;
    }
  return hval;
}

static const char * uri_scheme_list[] =
  {
    "", "",
    "tv",
    "sip",
    "sips",
    "", "",
    "service",
    "",
    "file",
    "", "",
    "webster",
    "lrq",
    "h323",
    "", "", "",
    "ftp",
    "sipt",
    "", "", "", "",
    "h320",
    "", "", "",
    "irc",
    "news",
    "", "", "", "",
    "http",
    "https",
    "", "", "",
    "h324",
    "", "", "",
    "sdp",
    "sipu",
    "wais",
    "telnet",
    "",
    "videotex",
    "fax",
    "",
    "tn3270",
    "mailto",
    "",
    "nntp",
    "",
    "finger",
    "",
    "pop",
    "pop3",
    "whois",
    "gopher",
    "whois++",
    "prospero",
    "smtp",
    "", "", "", "",
    "t120",
    "", "", "", "", "", "", "", "", "",
    "", "",
    "rlogin",
    "", "", "", "",
    "rwhois",
    "", "", "", "", "", "", "", "",
    "phone",
    "", "", "", "",
    "modem",
    "data"
  };

#ifdef __GNUC__
__inline
#endif
const char *
uri_scheme_exists (str, len)
     register const char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = uri_scheme_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = uri_scheme_list[key];

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1))
            return s;
        }
    }
  return 0;
}
