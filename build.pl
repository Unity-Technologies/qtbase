#!/usr/bin/env perl

use BuildOpenSSL;

use warnings;
use strict;
use Cwd qw[getcwd];
use File::Spec::Functions qw[canonpath];
use Getopt::Long;
use Params::Check qw[check];

my $launchVisualStudioEnv = '"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\vcvarsall.bat"';
my $root = getcwd ();
my $openSSL =  "$root\\openssl";
my %platforms = (
	MSWin32 => { 32 => 'x86', 64 => 'amd64' },
	darwin => {32 => 'macx-clang-32', 64 => 'macx-clang' }
 );

sub clean
{
	print "Cleaning\n";
	doSystemCommand ("git clean -xdf");
	doSystemCommand ("git reset --hard HEAD");
}

sub confugreLine
{
	my ($arch) = @_;
	my $os_name = $^O;
  my $platform = $platforms{$os_name}->{$arch};
	if ($os_name eq 'MSWin32')
	{
		return ("$launchVisualStudioEnv $platform && configure -platform win32-msvc2010 -prefix %CD%\\qtbase-$platform -opensource -confirm-license -no-opengl -no-icu -nomake examples -nomake tests -no-rtti -no-dbus -strip -openssl-linked OPENSSL_LIBS=\"-lssleay32 -llibeay32 -lgdi32 -luser32\" -I \"$openSSL\\openssl-$platform\\include\" -L \"$openSSL\\openssl-$platform\\lib\"");
	}
	elsif ($os_name eq 'darwin')
	{
		return ("./configure -platform $platform -prefix `pwd`/qtbase-$platform -opensource -confirm-license -no-icu -nomake examples -nomake tests -no-framework");
	}
	die ("Unknown platform $os_name");
}

sub makeInstallCommandLine
{
	my ($arch) = @_;
	my $os_name = $^O;
  my $platform = $platforms{$os_name}->{$arch};
	if ($os_name eq 'MSWin32')
	{
		return ("$launchVisualStudioEnv $platform && nmake install");
	}
	elsif ($os_name eq 'darwin')
	{
		return ("make && make install");
	}
	die ("Unknown platform $os_name");
}

sub doSystemCommand
{
    my $systemCommand = $_[0];
    my $returnCode = system ($systemCommand);
    if ($returnCode != 0) 
    { 
        die ("Failed executing [$systemCommand]\n"); 
    }
}

sub prepare
{
	my ($arch) = shift;
	print ("Preparing\n");

	my $os_name = $^O;
	if ($os_name eq 'MSWin32')
	{
		BuildOpenSSL::buildOpenSSL ($openSSL, $arch);
	}
}

sub configure
{
	my ($arch) = @_;
	print "\nConfiguring\n";
	my $cmd = confugreLine ($arch);
	doSystemCommand ($cmd);
}

sub make
{
	my ($arch) = @_;
	print "Making\n";
	my $cmd = makeInstallCommandLine ($arch);
	doSystemCommand ($cmd);
}

sub printUsage 
{
	print ("You must supply --arch\n");
	exit (1);
}

sub getArgs
{
	my ($options) = {
		qt_repo_dir => canonpath (getcwd ())
	};
	GetOptions ($options, 'arch=s', 'qtversion=s') or printUsage ();
	my $arg_scheme = {
		arch => { required => 1, allow => ['32', '64'] }
	};
	check ($arg_scheme, $options, 0) or printUsage ();
	if (scalar(@ARGV))
	{
		$options->{qt_repo_dir} = canonpath ($ARGV[0]);
    }
    return %{$options};
}

sub zip
{
	my ($arch) = @_;
	my $os_name = $^O;
	my $platform = $platforms{$os_name}->{$arch};
	if ($os_name eq 'MSWin32')
	{
		my $zipCmd = '"7z"';
		doSystemCommand("$zipCmd a -r build/builds.7z ./qtbase-$platform/*");
	}
	elsif ($os_name eq 'darwin')
	{
		my $zipCmd = '"./BuildTools/MacUtils/7za"';
		doSystemCommand("$zipCmd a -r build/builds.7z ./qtbase-$platform/*");
	}
	else
	{
		die ("Unknown platform $os_name");
	}
}

sub main
{
	my %params = getArgs ();
	clean ();
	prepare ($params{arch});
	configure ($params{arch});
	make ($params{arch});
	zip ($params{arch});
}

main();
