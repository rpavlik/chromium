#!/usr/bin/perl
#
# Program called as:
# trans_undef_symbols.pl <prefix> <dir> <library> [libraries] ...
#
# by Keith Jeffery <jeff2619@cs.uidaho.edu>
#

my $numArgs	= @ARGV;

if ($numArgs < 3) {
	die ("usage: trans_undef_symbols.pl <prefix> <dir> <library> [libraries] ...\n");
}

my $prefix	= shift @ARGV;
$prefix .= "_";
my $inputDir	= shift @ARGV;
my @libraries	= @ARGV;
my $OS		= $^O;

# either using gnm (SunOS) or nm
# either using gobjcopy (SunOS) or objcopy
my $nmCommand = $OS =~ /solaris/ ? "gnm" : "nm";
my $objCopyCommand = $OS =~ /solaris/ ? "gobjcopy" : "objcopy";

# checking to see if input directory exists
if (-e $inputDir) { # exists?
	if (!(-d $inputDir)) { # directory?
		die "$inputDir is not a directory.\n";
	}
} else {
	die "$inputDir does not exist.\n";
}

# adding a trailing / to $intputDir if necessary
if ($inputDir !~ /\/$/) {
	$inputDir .= "/";
}

# for every library
# nm -D --defined-only
# find those that start with the prefix
# strip the prefix and store in array
my @allSymbols;
my @librarySymbols;
my $library;
my $line;

foreach $library (@libraries) {
	# check to see if library exists
	if (!(-e $library)) {
		warn "Library $library doesn't exist.\n";
		next;
	}
	@librarySymbols = `$nmCommand -D --defined-only $library`;
	while (@librarySymbols) {
		$line = shift @librarySymbols;
		# we don't actually use $hex, it's just a place-holder
		my ($hex, $type, $name);
		chomp $line;
		($hex, $type, $name) = split ' ', $line;
		if ($type =~ /T|D|B/) {
			if ($name =~ /^$prefix/) {
				# This symbol HAS the prefix
				# now let's KILL it!
				$name =~ s/^$prefix//;
				push @allSymbols, $name;
			}
		}
	}
}

# for every .o file
# for every entry in array, entry
# objcopy --redefine-sym <entry>=<prefix_entry> <.oFile> <.oFile>

my @oFiles = <$inputDir*.o>;
my $objectCopyCall;
my $oFile;
my $newLine;

# constructing the objCopy call, except for the input and output files
$objectCopyCall = "$objCopyCommand";
foreach $line (@allSymbols) {
	$newLine = $prefix . $line;
	$objectCopyCall .= " --redefine-sym $line=$newLine";
}

my $outFile;
my $oFileName;
my $ret;

# calling the objCopy command with the input and output files, which are the same in this case
foreach $oFile (@oFiles) {
	$ret = system "$objectCopyCall $oFile $oFile";
	$ret >>= 8;	# divide return value by 256, only because the Perl documentation tells me to
	if ($ret) {
			warn "$objCopyCommand returned a system value of $ret\n";
	}
}
