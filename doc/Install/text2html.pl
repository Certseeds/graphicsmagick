#!/usr/bin/perl
    eval 'exec perl -S $0 "$@"'
    if $runnning_under_some_shell;
#
# txt2html.pl
# Convert raw text to something with a little HTML formatting
#
# Written by Seth Golub <seth@cs.wustl.edu>
#            http://www.cs.wustl.edu/~seth/txt2html/
#
# $Revision$
# $Date$
# $Author$
#
#
# $Log$
# Revision 1.10  1994/12/28  20:10:25  seth
#  * Added --extract, etc.
#
# Revision 1.9  94/12/13  15:16:23  15:16:23  seth (Seth Golub)
#  * Changed from #!/usr/local/bin/perl to the more clever version in
#    the man page.  (How did I manage not to read this for so long?)
#  * Swapped hrule & header back to handle double lines.  Why should
#    this order screw up headers?
#
# Revision 1.8  1994/11/30  21:07:03  seth
#  * put mail_anchor back in.  (Why did I take this out?)
#  * Finally added handling of lettered lists (ordered lists marked with
#    letters)
#  * Added title option (--title, -t)
#  * Shortline now looks at how long the line was before txt2html
#    started adding tags.   ($line_length)
#  * Changed list references to scalars where appropriate.  (@foo[0] -> $foo[0])
#  * Added untabify() to homogenize leading indentation for list
#    prefixes and functions that use line length
#  * Added "underline tolerance" for when underlines are not exactly the
#    same length as what they underline.
#  * Added error message for unrecognized options
#  * removed \w matching on --capstag
#  * Tagline now removes leading & trailing whitespace before tagging
#  * swapped order of caps & heading in main loop
#  * Cleaned up code for speed and to get rid of warnings
#  * Added more restrictions to something being a mail header
#  * Added indentation for lists, just to make the output more readable.
#  * Fixed major bug in lists: $OL and $UL were never set, so when a
#    list was ended "</UL>" was *always* used!
#  * swapped order of hrule & header to properly handle long underlines
#
# Revision 1.7  94/10/28  13:16:11  13:16:11  seth (Seth Golub)
#  * Added to comments in options section
#  * renamed blank to is_blank
#  * Page break is converted to horizontal rule <HR>
#  * moved usage subroutine up top so people who look through code see
#    it sooner
#
# Revision 1.6  94/10/28  12:43:46  12:43:46  seth (Seth Golub)
#  * Creates anchors at each heading
#
# Revision 1.5  94/07/14  17:43:59  17:43:59  seth (Seth Golub)
#  * Fixed minor bug in Headers
#  * Preformatting can be set to only start/stop when TWO lines of
#    [non]formatted-looking-text are encountered.  Old behavior is still
#    possible through command line options (-pb 1 -pe 1).
#  * Can preformat entire document (-pb 0) or disable preformatting
#    completely (-pe 0).
#  * Fixed minor bug in CAPS handling (paragraph breaks broke)
#  * Puts paragraph tags *before* paragraphs, not just between them.
#
# Revision 1.4  94/06/20  16:42:55  16:42:55  seth (Seth Golub)
#  * Allow ':' for numbered lists (e.g. "1: Figs")
#  * Whitespace at end of line will not start or end preformatting
#  * Mailmode is now off by default
#  * Doesn't break short lines if they are the first line in a list
#    item.  It *should* break them anyway if the next line is a
#    continuation of the list item, but I haven't dealt with this yet.
#  * Added action on lines that are all capital letters.  You can change
#    how these lines get tagged, as well as the minimum number of
#    consecutive capital letters required to fire off this action.
#
# Revision 1.3  94/05/17  15:58:58  15:58:58  seth (Seth Golub)
# * Tiny bugfix in unhyphenation
#
# Revision 1.2  94/05/16  18:15:16  18:15:16  seth (Seth Golub)
#  * Added unhyphenation
#
# Revision 1.1  94/05/16  16:19:03  16:19:03  seth (Seth Golub)
# Initial revision
#
#
# 1.02  Allow '-' in mail headers
#       Added handling for multiline mail headers
#
#
#
# Oscar Nierstrasz has a nice script for hypertextifying URLs.
# It is available at:
#   http://cui_www.unige.ch/ftp/PUBLIC/oscar/scripts/html.pl
#

#########################
# Configurable options
#

# [-s <n>    ] | [--shortline <n>                 ]
$short_line_length = 40;        # Lines this short (or shorter) must be
                                # intentionally broken and are kept
                                # that short. <BR>

# [-p <n>    ] | [--prewhite <n>                  ]
$preformat_whitespace_min = 5;  # Minimum number of consecutive leading
                                # whitespace characters to trigger
                                # preformatting.
                                # NOTE: Tabs are now expanded to
                                # spaces before this check is made.
                                # That means if $tab_width is 8 and
                                # this is 5, then one tab is expanded
                                # to 8 spaces, which is enough to
                                # trigger preformatting.

# [-pb <n>   ] | [--prebegin <n>                  ]
$preformat_trigger_lines = 2;   # How many lines of preformatted-looking
                                # text are needed to switch to <PRE>
                                # <= 0 : Preformat entire document
                                #    1 : one line triggers
                                # >= 2 : two lines trigger

# [-pe <n>   ] | [--preend <n>                    ]
$endpreformat_trigger_lines = 2; # How many lines of unpreformatted-looking
                                 # text are needed to switch from <PRE>
                                 # <= 0 : Never preformat within document
                                 #    1 : one line triggers
                                 # >= 2 : two lines trigger
# NOTE for --prebegin and --preend:
# A zero takes precedence.  If one is zero, the other is ignored.
# If both are zero, entire document is preformatted.


# [-r <n>    ] | [--hrule <n>                     ]
$hrule_min = 4;                 # Min number of ---s for an HRule.

# [-c <n>    ] | [--caps <n>                      ]
$min_caps_length = 3;           # min sequential CAPS for an all-caps line

# [-ct <tag> ] | [--capstag <tag>                 ]
$caps_tag = "STRONG";           # Tag to put around all-caps lines

# [-m/+m     ] | [--mail        / --nomail        ]
$mailmode = 0;                  # Deal with mail headers & quoted text

# [-u/+u     ] | [--unhyphenate / --nounhyphenate ]
$unhyphenation = 1;             # Enables unhyphenation of text.

# [-a <file> ] | [--append <file>                 ]
# [+a        ] | [--noappend                      ]
$append_file = 0;               # If you want something appended by
                                # default, put the filename here.
                                # The appended text will not be
                                # processed at all, so make sure it's
                                # plain text or decent HTML.  i.e. do
                                # not have things like:
                                #   Seth Golub <seth@cs.wustl.edu>
                                # but instead, have:
                                #   Seth Golub &lt;seth@cs.wustl.edu&gt;

# [-t <title>] | [--title <title>                 ]
$title = 0;                     # You can specify a title.
                                # Otherwise it won't put one in.

# [-ul <n>   ] | [--underlinelong <n>             ]
$underline_tolerance_long = 1;  # How much longer can underlines
                                # be and still be underlines?

# [-us <n>   ] | [--underlineshort <n>            ]
$underline_tolerance_short = 1; # How much shorter can underlines
                                # be and still be underlines?

# [-tw <n>   ] | [--tabwidth <n>                  ]
$tab_width = 8;                 # How many spaces equal a tab?


# [-iw <n>   ] | [--indent <n>                    ]
$indent_width = 2;              # Indents this many spaces for each
                                # level of a list

# [-/+e      ] | [--extract / --noextract         ]
$extract = 0;                   # Extract Mode (suitable for inserting)

# END OF CONFIGURABLE OPTIONS
########################################


########################################
# Definitions  (Don't change these)
#
$NONE       =   0;
$LIST       =   1;
$HRULE      =   2;
$PAR        =   4;
$PRE        =   8;
$END        =  16;
$BREAK      =  32;
$HEADER     =  64;
$MAILHEADER = 128;
$MAILQUOTE  = 256;
$CAPS       = 512;

$OL = 1;
$UL = 2;

sub usage
{
    $0 =~ s#.*/##;
    local($s) = " " x length($0);
    print STDERR <<EOF;

Usage: $0 [options]

where options are:
       $s [-v        ] | [--version                       ]
       $s [-h        ] | [--help                          ]
       $s [-s <n>    ] | [--shortline <n>                 ]
       $s [-p <n>    ] | [--prewhite <n>                  ]
       $s [-pb <n>   ] | [--prebegin <n>                  ]
       $s [-pe <n>   ] | [--preend <n>                    ]
       $s [-e/+e     ] | [--extract / --noextract         ]
       $s [-r <n>    ] | [--hrule <n>                     ]
       $s [-c <n>    ] | [--caps <n>                      ]
       $s [-ct <tag> ] | [--capstag <tag>                 ]
       $s [-m/+m     ] | [--mail     / --nomail           ]
       $s [-u/+u     ] | [--unhyphen / --nounhyphen       ]
       $s [-a <file> ] | [--append <file>                 ]
       $s [+a        ] | [--noappend                      ]
       $s [-t <title>] | [--title <title>                 ]
       $s [-tw <n>   ] | [--tabwidth <n>                  ]
       $s [-iw <n>   ] | [--indent <n>                    ]
       $s [-ul <n>   ] | [--underlinelong <n>             ]
       $s [-us <n>   ] | [--underlineshort <n>            ]

  More complete explanations of these options can be found in
  comments near the beginning of the script.

EOF
}


sub deal_with_options
{
    while ($ARGV[0] =~ /^[-+].+/)
    {
        if (($ARGV[0] eq "-r" || $ARGV[0] eq "--hrule") &&
            $ARGV[1] =~ /^%d+$/)
        {
            $hrule_min = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-s" || $ARGV[0] eq "--shortline") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $short_line_length = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-p" || $ARGV[0] eq "--prewhite") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $preformat_whitespace_min = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-pb" || $ARGV[0] eq "--prebegin") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $preformat_trigger_lines = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-pe" || $ARGV[0] eq "--preend") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $endpreformat_trigger_lines = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-e" || $ARGV[0] eq "--extract"))
        {
            $extract = 1;
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "+e" || $ARGV[0] eq "--noextract"))
        {
            $extract = 0;
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-c" || $ARGV[0] eq "--caps") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $min_caps_length = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-ct" || $ARGV[0] eq "--capstag") &&
            $ARGV[1])
        {
            $caps_tag = $ARGV[1];
            shift @ARGV;
            next;
        }

        if ($ARGV[0] eq "-m" || $ARGV[0] eq "--mail")
        {
            $mailmode = 1;
            next;
        }

        if ($ARGV[0] eq "+m" || $ARGV[0] eq "--nomail")
        {
            $mailmode = 0;
            next;
        }

        if ($ARGV[0] eq "-u" || $ARGV[0] eq "--unhyphen")
        {
            $unhyphenation = 1;
            next;
        }

        if ($ARGV[0] eq "+u" || $ARGV[0] eq "--nounhyphen")
        {
            $unhyphenation = 0;
            next;
        }

        if (($ARGV[0] eq "-a" || $ARGV[0] eq "--append") &&
            $ARGV[1])
        {
            if (-r $ARGV[1]) {
                $append_file = $ARGV[1];
            } else {
                print STDERR "Can't find or read $ARGV[1].\n";
            }
            shift @ARGV;
            next;
        }

        if ($ARGV[0] eq "+a" || $ARGV[0] eq "--noappend")
        {
            $append_file = 0;
            next;
        }

        if (($ARGV[0] eq "-t" || $ARGV[0] eq "--title") &&
            $ARGV[1])
        {
            $title = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-ul" || $ARGV[0] eq "--underlinelong") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $underline_tolerance_long = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-us" || $ARGV[0] eq "--underlineshort") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $underline_tolerance_short = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-tw" || $ARGV[0] eq "--tabwidth") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $tab_width = $ARGV[1];
            shift @ARGV;
            next;
        }

        if (($ARGV[0] eq "-iw" || $ARGV[0] eq "--indentwidth") &&
            $ARGV[1] =~ /^\d+$/)
        {
            $indent_width = $ARGV[1];
            shift @ARGV;
            next;
        }

        if ($ARGV[0] eq "-v" || $ARGV[0] eq "--version")
        {
            print '$Header: /users/hilco/seth/projects/txt2html/txt2html.pl,v 1
.10 1994/12/28 20:10:25 seth Exp seth $ ';
            print "\n";
            exit;
        }

        if ($ARGV[0] eq "-h" || $ARGV[0] eq "--help")
        {
            &usage;
            exit;
        }

        print STDERR "Unrecognized option: $ARGV[0]\n";
        print STDERR " or bad parameter: $ARGV[1]\n" if($ARGV[1]);

        &usage;
        exit(1);

    } continue {

        shift @ARGV;
    }

    $preformat_trigger_lines = 0 if ($preformat_trigger_lines < 0);
    $preformat_trigger_lines = 2 if ($preformat_trigger_lines > 2);

    $endpreformat_trigger_lines = 1 if ($preformat_trigger_lines == 0);
    $endpreformat_trigger_lines = 0 if ($endpreformat_trigger_lines < 0);
    $endpreformat_trigger_lines = 2 if ($endpreformat_trigger_lines > 2);

    $underline_tolerance_long  = 0 if $underline_tolerance_long < 0;
    $underline_tolerance_short = 0 if $underline_tolerance_short < 0;
}

sub is_blank
{
    return $_[0] =~ /^\s*$/;
}

sub escape
{
    $line =~ s/&/&amp;/g;
    $line =~ s/>/&gt;/g;
    $line =~ s/</&lt;/g;
    $line =~ s/\014/\n<HR>\n/g; # Linefeeds become horizontal rules
}

sub hrule
{
    if ($line =~ /^\s*([-_~=\*]\s*){$hrule_min,}$/)
    {
        $line = "<HR>\n";
        $prev =~ s/<p>//;
        $line_action |= $HRULE;
    }
}

sub shortline
{
    if (!($mode & $PRE) &&
        !&is_blank($line) &&
        ($line_length < $short_line_length) &&
        !&is_blank($nextline) &&
        !($line_action & ($HEADER | $HRULE | $BREAK | $LIST)))
    {
        $line =~ s/$/<BR>/;
        $line_action |= $BREAK;
    }
}

sub mailstuff
{
    if ((($line =~ /^\w*&gt/) || # Handle "FF> Werewolves."
         ($line =~ /^\w*\|/))&&  # Handle "Igor| There wolves."
        !&is_blank($nextline))
    {
        $line =~ s/$/<BR>/;
        $line_action |= $BREAK | $MAILQUOTE;
    } elsif (($line =~ /^[\w\-]*:/) # Handle "Some-Header: blah"
             && (($previous_action & $MAILHEADER) || &is_blank($prev))
             && !&is_blank($nextline))
    {
        &anchor_mail if !($previous_action & $MAILHEADER);
        $line =~ s/$/<BR>/;
        $line_action |= $BREAK | $MAILHEADER;
    } elsif (($line =~ /^\s+\S/) &&   # Handle multi-line mail headers
             ($previous_action & $MAILHEADER) &&
             !&is_blank($nextline))
    {
        $line =~ s/$/<BR>/;
        $line_action |= $BREAK | $MAILHEADER;
    }
}

sub paragraph
{
    $prev .= "<p>\n";
    $line_action |= $PAR;
}

sub listprefix
{
    local($line) = @_;
    local($prefix, $number, $rawprefix);

    return (0,0,0) if (!($line =~ /^\s*[-=\*o]\s+\S/ ) &&
                       !($line =~ /^\s*(\d+|[a-zA-Z])[\.\)\]:]\s+\S/ ));

    ($number) = $line =~ /^\s*(\d+|[a-zA-Z])/;

    # That slippery exception of "o" as a bullet
    # (This ought to be determined more through the context of what lists
    #  we have in progress, but this will probably work well enough.)
    if($line =~ /^\s*o\s/)
    {
        $number = 0;
    }

    if ($number)
    {
        ($rawprefix) = $line =~ /^(\s*(\d+|[a-zA-Z]).)/;
        $prefix = $rawprefix;
        $prefix =~ s/(\d+|[a-zA-Z])//;  # Take the number out
    } else {
        ($rawprefix) = $line =~ /^(\s*[-=o\*].)/;
        $prefix = $rawprefix;
    }
    ($prefix, $number, $rawprefix);
}

sub startlist
{
    local($prefix, $number, $rawprefix) = @_;

    $listprefix[$listnum] = $prefix;
    if($number)
    {
        # It doesn't start with 1,a,A.  Let's not screw with it.
        if (($number != 1) && ($number ne "a") && ($number ne "A"))
        {
            return;
        }
        $prev .= "$list_indent<OL>\n";
        $list[$listnum] = $OL;
    } else {
        $prev .= "$list_indent<UL>\n";
        $list[$listnum] = $UL;
    }
    $listnum++;
    $list_indent = " " x $listnum x $indent_width;
    $line_action |= $LIST;
    $mode |= $LIST;
}


sub endlist                     # End N lists
{
    local($n) = @_;
    for(; $n > 0; $n--, $listnum--)
    {
        $list_indent = " " x ($listnum-1) x $indent_width;
        if($list[$listnum-1] == $UL)
        {
            $prev .= "$list_indent</UL>\n";
        } elsif($list[$listnum-1] == $OL)
        {
            $prev .= "$list_indent</OL>\n";
        } else
        {
            print STDERR "Encountered list of unknown type\n";
        }
    }
    $line_action |= $END;
    $mode ^= ($LIST & $mode) if (!$listnum);
}

sub continuelist
{
    $line =~ s/^\s*[-=o\*]\s*/$list_indent<LI> / if $list[$listnum-1] == $UL;
    $line =~ s/^\s*(\d+|[a-zA-Z]).\s*/$list_indent<LI> /    if $list[$listnum-1
] == $OL;
    $line_action |= $LIST;
}

sub liststuff
{
    local($i);

    local($prefix, $number, $rawprefix) = &listprefix($line);

    $i = $listnum;
    if (!$prefix)
    {
        return if !&is_blank($prev); # inside a list item

        # This ain't no list.  We'll want to end all of them.
        return if !($mode & $LIST);     # This just speeds up the inevitable
        $i = 0;
    } else
    {
        # Maybe we're going back up to a previous list
        $i-- while (($prefix ne $listprefix[$i-1]) && ($i >= 0));
    }

    if (($i >= 0) && ($i != $listnum))
    {
        &endlist($listnum - $i);
    } elsif (!$listnum || $i != $listnum)
    {
        &startlist($prefix, $number, $rawprefix);
    }

    &continuelist($prefix, $number, $rawprefix) if ($mode & $LIST);
}

sub endpreformat
{
    if(!($line =~ /\s{$preformat_whitespace_min,}\S+/) &&
       ($endpreformat_trigger_lines == 1 ||
        !($nextline =~ /\s{$preformat_whitespace_min,}\S+/)))
    {
        $prev =~ s#$#\n</PRE>#;
        $mode ^= ($PRE & $mode);
        $line_action |= $END;
    }
}

sub preformat
{
    if($preformat_trigger_lines == 0 ||
       (($line =~ /\s{$preformat_whitespace_min,}\S+/) &&
        ($preformat_trigger_lines == 1 ||
         $nextline =~ /\s{$preformat_whitespace_min,}\S+/)))
    {
        $line =~ s/^/<PRE>\n/;
        $prev =~ s/<p>//;
        $mode |= $PRE;
        $line_action |= $PRE;
    }
}

sub make_new_anchor
{
    $anchor++;
    $anchor;
}

sub anchor_mail
{
    local($text) = $line =~ /\S+: *(.*) *$/;
    local($anchor) = &make_new_anchor($text);
    $line =~ s/(.*)/<A NAME="$anchor">$1<\/A>/;
}

sub anchor_heading
{
    local($heading) = @_;
    local($anchor) = &make_new_anchor($heading);
    $line =~ s/(<H.>.*<\/H.>)/<A NAME="$anchor">$1<\/A>/;
}

sub heading
{
    local($hindent, $heading) = $line =~ /^(\s*)(.+)$/;
    $hindent = 0;               # This isn't used yet, but Perl warns of
                                # "possible typo" if I declare a var
                                # and never reference it.

    # This is now taken care of in main()
#    $heading =~ s/\s+$//;      # get rid of trailing whitespace.

    local($underline) = $nextline =~ /^\s*(\S+)\s*$/;

    if((length($heading) > (length($underline) + $underline_tolerance_short))
       || (length($heading) < (length($underline) -$underline_tolerance_long)))
    {
        return;
    }

#    $underline =~ s/(^.).*/$1/;     # Could I do this any less efficiently?
    $underline = substr($underline,0,1);

    local($hlevel);
    $hlevel = 1 if $underline eq "*";
    $hlevel = 2 if $underline eq "=";
    $hlevel = 3 if $underline eq "+";
    $hlevel = 4 if $underline eq "-";
    $hlevel = 5 if $underline eq "~";
    $hlevel = 6 if $underline eq ".";
    return if !$hlevel;

    $nextline = <STDIN>;        # Eat the underline
    &tagline("H${hlevel}");
    &anchor_heading($heading);
    $line_action |= $HEADER;
}

sub unhyphenate
{
    local($second);

    # This looks hairy because of all the quoted characters.
    # All I'm doing is pulling out the word that begins the next line.
    # Along with it, I pull out any punctuation that follows.
    # Preceding whitespace is preserved.  We don't want to screw up
    # our own guessing systems that rely on indentation.
    ($second) = $nextline =~ /^\s*([a-zA-Z]+[\)\}\]\.,:;\'\"\>]*\s*)/; # "
    $nextline =~ s/^(\s*)[a-zA-Z]+[\)\}\]\.,:;\'\"\>]*\s*/$1/; # "
    # (The silly comments are for my less-than-perfect code hilighter)

    $line =~ s/\-\s*$/$second/;
    $line .= "\n";
}

sub untabify
{
    local($oldws) = $line =~ /^([ \011]+)/;
    local($oldlen) = (length($oldws));

    local($i, $column);
    for($i=0, $column = 0; $i < $oldlen; $i++)
    {
        if(substr($oldws, $i, 1) eq " ")
        {                       # Space
            $column++;
        } else {                # Tab
            $column += $tab_width - ($column % $tab_width);
        }
    }
    $line = (" " x $column) . substr($line, $oldlen);
}

sub tagline
{
    local($tag) = @_;
    $line =~ s/^\s*(.*)\s*$/<$tag>$1<\/$tag>\n/;
}

sub caps
{
    if($line =~ /^[^a-z<]*[A-Z]{$min_caps_length,}[^a-z<]*$/)
    {
        &tagline($caps_tag);
        $line_action |= $CAPS;
    }
}



sub main
{
    &deal_with_options;

    if(!$extract)
    {
        print "<HTML>\n";
        print "<HEAD>\n";

        # It'd be nice if we could guess a title from the first header,
        # but even that would be too late if we're doing this in one pass.
        print "<TITLE>$title</TITLE>\n" if($title);

        print "</HEAD>\n";
                                
        print "<BODY TEXT=\"#000000\" BGCOLOR=\"#FFFFFF\" LINK=\"#0000EE\" VLINK=\"#551A8B\" ALINK=\"#FF0000\">\n";
    }

    $prev     = "";
    $line     = <STDIN>;
    $nextline = <STDIN>;
    do
    {
        $line =~ s/[ \011]*$//; # Chop trailing whitespace

        &untabify;              # Change leading whitespace into spaces

        $line_length = length($line); # Do this before tags go in

        &escape;

        &endpreformat if (($mode & $PRE) && ($preformat_trigger_lines != 0));

        &hrule if !($mode & $PRE);

        &heading   if (!($mode & $PRE) &&
                       $nextline =~ /^\s*[=\-\*\.~\+]+$/);

        &caps if  !($mode & $PRE);

        &liststuff if (!($mode & $PRE) &&
                       !&is_blank($line));

        &mailstuff if ($mailmode &&
                       !($mode & $PRE) &&
                       !($line_action & $HEADER));

        &preformat if (!($line_action & ($HEADER | $LIST | $MAILHEADER)) &&
                       !($mode & ($LIST | $PRE)) &&
                       ($endpreformat_trigger_lines != 0));

        &paragraph if ((&is_blank($prev) || ($line_action & $END)) &&
                       !&is_blank($line) &&
                       !($mode & ($LIST | $PRE)) && # paragraphs in lists
                                                    # *should* be allowed.
                       (!$line_action ||
                        ($line_action & ($CAPS | $END | $MAILQUOTE))));

        &shortline;

        &unhyphenate if ($unhyphenation &&
                         ($line =~ /[a-zA-Z]\-$/) && # ends in hyphen
                         # next line starts w/letters
                         ($nextline =~ /^\s*[a-zA-Z]/) &&
                         !($mode & ($PRE | $HEADER | $MAILHEADER | $BREAK)));


        # Print it out and move on.

        print $prev;

        if (!&is_blank($nextline))
        {
            $previous_action = $line_action;
            $line_action     = $NONE;
        }

        $prev = $line;
        $line = $nextline;
        $nextline = <STDIN>;
    } until (!$nextline && !$line && !$prev);

    $prev = "";
    &endlist($listnum) if ($mode & $LIST); # End all lists
    print $prev;

    print "\n";

    print "</PRE>\n" if ($mode & $PRE);

    if ($append_file)
    {
        if(-r $append_file)
        {
            open(APPEND, $append_file);
            print while <APPEND>;
        } else {
            print STDERR "Can't find or read file $append_file to append.\n";
        }
    }

    if(!$extract)
    {
        print "</BODY>\n";
        print "</HTML>\n";
    }
}

&main();


