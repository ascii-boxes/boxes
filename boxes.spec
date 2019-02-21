%define name	boxes
%define version	1.3
%define release	0
%define prefix	/usr
%define cfgfile	%prefix/share/boxes

%define nvr	%{name}-%{version}-%{release}
%define defbr	/var/tmp/%{nvr}-build

name:		%name
version:	%version
release:	%release

vendor:		Thomas Jensen <boxes@thomasjensen.com>
packager:	Thomas Jensen <boxes@thomasjensen.com>

summary:	Command line ASCII boxes unlimited
group:		Applications/Text

copyright:	GPL Version 2
URL:		https://boxes.thomasjensen.com/

source:		http://github.com/ascii-boxes/%{name}/archive/v%{version}.tar.gz
buildroot:	%defbr

%description
"boxes" can draw all kinds of boxes around its input text, ranging from a C
comment box to complex ASCII art. These boxes may also be removed, even if
they have been badly damaged by editing of the text inside. Since boxes may
be open on any side, "boxes" can also be used to create regional comments in
any programming language. With the help of an editor macro or mapping,
damaged boxes can easily be repaired. New box designs of all sorts can
easily be added and shared by appending to a free format configuration file.

###########################################################################
# useful macros
###########################################################################
%define cleanroot [ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" = "%defbuildroot" ] && rm -rf "$RPM_BUILD_ROOT"
%define restorebins [ -f $RPM_BUILD_DIR/$RPM_PACKAGE_NAME.cpio ] && cpio -iv -I $RPM_BUILD_DIR/$RPM_PACKAGE_NAME.cpio
###########################################################################

%prep
%setup

%build
rm doc/boxes.1
rm src/boxes.h
make GLOBALCONF=%cfgfile

%install
mkdir -p $RPM_BUILD_ROOT/%prefix/bin
mkdir -p $RPM_BUILD_ROOT/%prefix/share/man/man1

install -m 0755 src/boxes	$RPM_BUILD_ROOT/%prefix/bin
install -m 0644 doc/boxes.1 	$RPM_BUILD_ROOT/%prefix/share/man/man1
install -m 0644 boxes-config 	$RPM_BUILD_ROOT/%cfgfile

# write filelisting to /tmp
find "$RPM_BUILD_ROOT" -type f -printf "/%P\n" > /tmp/FILES-%nvr

%clean
# delete stuff
[ "$RPM_BUILD_ROOT" = "%defbr" -a -d "%defbr" ] && rm -rf %defbr

%files
%defattr(-, root, root)
/usr/bin/boxes
/usr/share/man/man1/boxes.1.gz
%config /usr/share/boxes
%doc COPYING README
