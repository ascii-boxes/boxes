#!/usr/bin/perl -w
#
#  Author:       Thomas Jensen <boxes@thomasjensen.com>
#  Date created: July 12, 1999 (Monday, 13:14h)
#  Language:     Perl 5
#  Version:      $Id: create_changelog.pl,v 1.9 2012/10/19 18:44:25 tsjensen Exp $
#
#  History:
#
#    $Log: create_changelog.pl,v $
#    Revision 1.9  2012/10/19 18:44:25  tsjensen
#    Added milestone for version 1.1.1
#
#    Revision 1.8  2012/10/19 16:42:10  tsjensen
#    Include files in non-src subfolders in the list of files at the end of the created document
#
#    Revision 1.7  2006/07/23 18:01:01  tsjensen
#    Added milestone for version 1.1
#    Changed footer include to reflect new filename
#    Added time zone handling to rlog calls, which gives us local checkin times
#
#    Revision 1.6  2006-07-12 19:30:40+02  tsjensen
#    Merged with newly discovered newest version
#
#    Revision 1.4  1999/09/10 17:33:32  tsjensen
#    Renamed current snapshot archive file to boxes-SNAP-latest.tar.gz
#    Added "milestones": Draw horizontal ruler at particular times (v1 out etc.)
#
#    Revision 1.3  1999/08/18 18:41:41  tsjensen
#    Added HTML conversions for '&' and eacute
#    Removing path from file name
#    Changed introductory text
#
#    Revision 1.2  1999/07/12 18:03:14  tsjensen
#    Moved "List of Files" from top to bottom of page
#    Many modifications on the page's appearance
#    In log messages, lines which start with a lower case character are
#    not preceded by a <BR>, thus concatenating lines which belong together.
#
#    Revision 1.1  1999/07/12 12:28:46  tsjensen
#    Initial revision
#______________________________________________________________________________
#==============================================================================


@files = @ARGV;
$#files >= 0 or die "no input files";

%milestones = (
    "2012/10/19 17:33:21" => "VERSION 1.1.1 RELEASED",
    "2006/07/23 18:28:13" => "VERSION 1.1 RELEASED",
    "2000/03/18 01:51:43" => "VERSION 1.0.1 RELEASED",
    "1999/08/22 13:37:27" => "VERSION 1.0 RELEASED",
    "1999/06/25 20:52:28" => "FIRST BETA RELEASED"
);

my $tz = `date +'%z' | cut -c -3`;
chomp($tz);

print '<HTML>

<HEAD>
    <TITLE>boxes - Change Log</TITLE>
    <META NAME="Author" CONTENT="Thomas Jensen">
    <META HTTP-EQUIV="Content-type"
          CONTENT="text/html; charset=iso-8859-1">
    <META NAME="Robots" CONTENT="index,follow">
</HEAD>

<BODY TEXT="#000000" LINK="#0000FF" VLINK="#C00080" BGCOLOR="#FFFFFF">

This page is automatically updated whenever a few of the <A
HREF="#flist">files listed below</A> are checked in, so it is very
up-to-date. It might already show changes which are not even in the <A
HREF="download/boxes-SNAP-latest.tar.gz">current snapshot</A> yet.

<H1>Chronological Change Log</H1>

<DL>
';

%cl = ();

foreach $dat (sort @files) {
    @tmp = `rlog -z$tz $dat`;
    $start = 0;

    $rev = "";
    $auth = "";
    $datum = "";
    @desc = ();

    for ($i=0; $i<=$#tmp; ++$i) {

        if ($start == 0) {
            next unless $tmp[$i] =~ /^----------------------------$/;
            $start = 1;
            next;
        }

        if ($tmp[$i] =~ /^date: ([^\+]*)\+/) {
            $datum = $1;
            $datum =~ tr/-/\//;
        }
        if ($tmp[$i] =~ /^revision ([0-9]\.[0-9]+)/) {
            $rev = $1;
        }
        if ($tmp[$i] =~ /author: ([^;]*);/) {
            $auth = $1;
        }
        if ($tmp[$i] !~ /^revision ([0-9]\.[0-9]+)/
                && $tmp[$i] !~ /^date: ([^;]*);/) {
            $tmp[$i] =~ s/&/\&amp;/g;
            $tmp[$i] =~ s/</\&lt;/g;
            $tmp[$i] =~ s/>/\&gt;/g;
            $tmp[$i] =~ s/ü/\&uuml;/g;
            $tmp[$i] =~ s/ö/\&ouml;/g;
            $tmp[$i] =~ s/ä/\&auml;/g;
            $tmp[$i] =~ s/Ü/\&Uuml;/g;
            $tmp[$i] =~ s/Ö/\&Ouml;/g;
            $tmp[$i] =~ s/Ä/\&Auml;/g;
            $tmp[$i] =~ s/ß/\&szlig;/g;
            $tmp[$i] =~ s/é/\&eacute;/g;
            $tmp[$i] =~ s/"/\&quot;/g;
            chop $tmp[$i];
            if ($tmp[$i] =~ /^Initial revision$/) {
                $tmp[$i] = "<FONT COLOR=\"#C00000\">".$tmp[$i]."</FONT>";
            }
            if (@desc > 0 && $tmp[$i] !~ /^[a-zäöü]/ && $tmp[$i] !~ /^[-=]{10}/) {
                $tmp[$i] = "<BR>".$tmp[$i];
            }
            push @desc, $tmp[$i]."\n";
        }

        if ($tmp[$i] =~ /^----------------------------$/
            || $tmp[$i] =~ /^===============================================/)
        {
            $desc[$#desc] = "";
            $simpleName = $dat;
            $simpleName =~ s/^.*\///;
            $cl{$datum."; <STRONG>".$simpleName." ".$rev."</STRONG> (".$auth.")"} = [ @desc ];
            @desc = ();
        }
    }
}


foreach $line (reverse sort keys %cl) {
    foreach (keys %milestones) {
        if ($line =~ /^$_/) {
            print "</DL>\n\n<HR>\n<CENTER><STRONG><FONT SIZE=\"-1\">";
            print $milestones{$_}, "</FONT></STRONG></CENTER>\n";
            print "<HR>\n\n<DL>\n";
        }
    }
    print "<DT><TT>$line</TT>\n";
    print "<DD><TT>", @{$cl{$line}}, "</TT><P>\n\n";
}



print '</DL>


<HR>
<H2><A NAME="flist">List of Files</A></H2>

<DL>
';

foreach $dat (sort @files) {
    next unless -r $dat;
    @tmp = `rlog $dat`;

    $dat =~ s/^.*\///;
    
    @foo = ();
    $start = 0;
    @desc = ();

    @foo = grep /^head: /, @tmp;
    $rev = $foo[0];
    $rev =~ s/^head: //;
    chop $rev;
    
    for ($i=0; $i<$#tmp; ++$i) {
        if ($start) {
            if ($start == 2) {
                last;
            }
            else {
                if ($tmp[$i] =~ /^----------------------------$/) {
                    $start = 2;
                }
                else {
                    push @desc, $tmp[$i];
                }
            }
        }
        else {
            $start = 1 if ($tmp[$i] =~ /^description:$/);
        }
    }

    print "<DT><TT>$dat</TT> <i>($rev)</i>\n";
    print "<DD>", @desc;
}


print '</DL>


<!--#include file="footer.inc.shtml" -->

</BODY>
</HTML>
';


#EOF                                                   vim: set sw=4:
