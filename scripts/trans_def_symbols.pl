#!/usr/bin/perl
# 
# Program called as:
# trans_def_symbols <prefix> <lib> <input_dir> <output_dir>
#
# by Keith Jeffery <jeff2619@cs.uidaho.edu>
#

my $numArgs	= @ARGV;

if ($numArgs != 4) {
	die ("$0 Error:\n\tUsage: trans_def_symbols <prefix> <lib> <input_dir> <output_dir>\n");
}

my $prefix    = $ARGV[0] . "_";
my $lib       = $ARGV[1];
my $inputDir  = $ARGV[2];
my $outputDir = $ARGV[3];

my $OS        = `uname`; 

# checking to see if input directory exists
if (-e $inputDir) { # exists?
	if (!(-d $inputDir)) { # directory?
		die "$0 Error:\n\t\"$inputDir\" exists, but is not a directory.\n";
	}
} else {
	die "$0 Error:\n\tDirectory \"$inputDir\" does not exist.\n";
}

# checking to see if output directory exists
if (-e $outputDir) { # exists?
	if (!(-d $outputDir)) { # is it really a directory?
		die "$0 Error:\n\t\"$outputDir\" already exists, but it is not a directory.\n";
	}
} else {
	mkdir($outputDir, 0755) || die "$0 Error:\n\tCannot create directory \"$outputDir\": $!";
}

# checking to see if library exists
if (!(-e $lib)) {
	die "$0 Error:\n\tInput library \"$lib\" does not exist.\n";
}

# either using gnm (SunOS) or nm 
# either using gobjcopy (SunOS) or objcopy 
my $nmCommand = $OS =~ /SunOS/ ? "gnm" : "nm";
my $objCopyCommand = $OS =~ /SunOS/ ? "gobjcopy" : "objcopy";

# adding a trailing / to $intputDir if necessary
if ($inputDir !~ /\/$/) {
	$inputDir .= "/";
}

# adding a trailing / to $outputDir if necessary
if ($outputDir !~ /\/$/) {
	$outputDir .= "/";
}

# run nm on $lib
# nm -g -D --defined-only $lib
# Grab lines that are T, D, or B
# Store the names in an array

my @nmResults = `$nmCommand -D --defined-only $lib`;
my (@storedNames, $line, $hex, $type, $name);
my $line;
foreach $line (@nmResults) {
	chomp $line;
	# we don't actually use $hex, it's just a place-holder
	($hex, $type, $name) = split ' ', $line;
	if ($type =~ /T|D|B/) {
		push @storedNames, $name;
	}
}

my $objectCopyCall;
my $newLine;

# constructing the objCopy call, except for the input and output files
$objectCopyCall = "$objCopyCommand";
foreach $line (@storedNames) {
	$newLine = $prefix . $line;
	$objectCopyCall .= " --redefine-sym $line=$newLine";
}

# for *.o in input_dir
# objcopy --redefine-sym [name from array]=[prefix_name from array] <input_file> <output_dir/input_file>
my @oFiles = <$inputDir*.o>;

my $outFile;
my $oFile;
my $ret;

# calling the objCopy command with the input and output files
foreach $oFile (@oFiles) {
  $oFile =~ /\/([\w\.]+)$/;
	$outFile = $outputDir . $1;
  
  # for debug
  #print "Would be calling $objectCopyCall $oFile $outFile";

  $ret = system "$objectCopyCall $oFile $outFile";
	$ret >>= 8;	# divide return value by 256, only because the Perl documentation tells me to
	if ($ret) {
			warn "$0 Warning:\n\t$objCopyCommand returned a system value of $ret\n";
	}
}

