#!/local/bin/perl5 -w

# Author: Thomas Jensen <tsjensen@stud.informatik.uni-erlangen.de>
# $Id$
# $Log$

@files = @ARGV;
$#files >= 0 or die "no input files";

print '<HTML>

<HEAD>
    <TITLE>boxes - Change Logs</TITLE>
    <META NAME="Author" CONTENT="Thomas Jensen">
    <META HTTP-EQUIV="Content-type"
          CONTENT="text/html; charset=iso-8859-1">
    <META NAME="Robots" CONTENT="index,follow">
</HEAD>

<BODY TEXT="#000000" LINK="#0000FF" VLINK="#C00080" BGCOLOR="#FFFFFF">

<H1><TT>Boxes</TT> Change Log</H1>

This page is an automatically generated chronologically sorted change log of
the files listed below.

<H2>List of Files</H2>

<DL>
';


foreach $dat (sort @files) {
    next unless -r $dat;
    @tmp = `rlog $dat`;

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

<H2>Chronological Change Log</H2>

<DL>
';

%cl = ();

foreach $dat (sort @files) {
    @tmp = `rlog $dat`;
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

        if ($tmp[$i] =~ /^date: ([^;]*);/) {
            $datum = $1;
        }
        if ($tmp[$i] =~ /^revision ([0-9]\.[0-9]+)/) {
            $rev = $1;
        }
        if ($tmp[$i] =~ /author: ([^;]*);/) {
            $auth = $1;
        }
        if ($tmp[$i] !~ /^revision ([0-9]\.[0-9]+)/
                && $tmp[$i] !~ /^date: ([^;]*);/) {
            $tmp[$i] =~ s/</\&lt;/g;
            $tmp[$i] =~ s/>/\&gt;/g;
            $tmp[$i] =~ s/ü/\&uuml;/g;
            $tmp[$i] =~ s/ö/\&ouml;/g;
            $tmp[$i] =~ s/ä/\&auml;/g;
            $tmp[$i] =~ s/Ü/\&Uuml;/g;
            $tmp[$i] =~ s/Ö/\&Ouml;/g;
            $tmp[$i] =~ s/Ä/\&Auml;/g;
            $tmp[$i] =~ s/ß/\&szlig;/g;
            $tmp[$i] =~ s/"/\&quot;/g;
            chop $tmp[$i];
            if ($tmp[$i] =~ /^Initial revision$/) {
                $tmp[$i] = "<FONT COLOR=\"#C00000\">".$tmp[$i]."</FONT>";
            }
            push @desc, $tmp[$i]."<BR>\n";
        }

        if ($tmp[$i] =~ /^----------------------------$/
            || $tmp[$i] =~ /^===============================================/)
        {
            $desc[$#desc] = "";
            $cl{$datum.";  <STRONG>".$dat." rev. ".$rev."</STRONG>  (".$auth.")"} = [ @desc ];
            @desc = ();
        }
    }
}


foreach $line (reverse sort keys %cl) {
    print "<DT><TT>$line</TT>\n";
    print "<DD><TT>", @{$cl{$line}}, "</TT><P>\n\n";
}


print "</DL>\n\n", '<!--#include file="footer.inc.html" -->

</BODY>
</HTML>
';


#EOF
