#!/usr/bin/env perl

use lib ".";
use BuildOpenSSL;

use warnings;
use strict;
use Cwd qw[getcwd];
use File::Copy;
use File::Path qw[make_path];
use File::Spec::Functions qw[canonpath];
use Getopt::Long;
use Params::Check qw[check];

my $launchVisualStudioEnv = '"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\vcvarsall.bat"';
my $root = getcwd ();
my $openSSL =  "$root\\openssl";
my %platforms = (
	MSWin32 => { 32 => 'x86', 64 => 'amd64' },
	darwin => { 32 => 'macx-clang-32', 64 => 'macx-clang' },
	linux => { 32 => 'x86', 64 => 'amd64' }
);

sub clean
{
	print "Cleaning\n";
	doSystemCommand ("git clean -xdf");
	#doSystemCommand ("git reset --hard HEAD");
}

sub confugreLine
{
	my ($arch) = @_;
	my $os_name = $^O;
	my $platform = $platforms{$os_name}->{$arch};
	if ($os_name eq 'MSWin32')
	{
		return ("$launchVisualStudioEnv $platform 10.0.16299.0 && configure -platform win32-msvc -prefix %CD%\\qtbase-$platform -opensource -confirm-license -no-opengl -no-icu -nomake examples -nomake tests -no-dbus -no-harfbuzz -strip -openssl-linked -I \"$openSSL\\openssl-$platform\\include\" -L \"$openSSL\\openssl-$platform\\lib\"");
	}
	elsif ($os_name eq 'darwin')
	{
		return ("./configure -platform $platform -prefix `pwd`/qtbase-$platform -opensource -confirm-license -no-icu -nomake examples -nomake tests -no-framework -qt-pcre");
	}
	elsif ($os_name eq 'linux')
	{
		$openSSL = "$root/openssl";
		return ("./configure -prefix ./qtbase-$platform -v -c++std c++11 -opensource -confirm-license -no-icu -qt-xcb -I/usr/include/xcb -I$openSSL/openssl-$platform/include -L$openSSL/openssl-$platform/lib -L/usr/lib/x86_64-linux-gnu -nomake tests -nomake examples -no-harfbuzz -qt-pcre -qt-libpng -openssl-linked --enable-shared -feature-freetype -fontconfig -recheck-all");
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
		return ("$launchVisualStudioEnv $platform 10.0.16299.0 && nmake && nmake install");
	}
	elsif ($os_name eq 'darwin')
	{
		return ("make -j`nproc` && make install");
	}
	elsif ($os_name eq 'linux')
	{
		return ("make -j`nproc` && make install");
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
	my $platform = $platforms{$os_name}->{$arch};

	if ($os_name eq 'MSWin32')
	{
		#pretty awful hack, the vcvarsall.bat script doesn't set up paths correctly to make rc.exe available
		#so we add this to the path so that Qt configure script can find it.
		$ENV{PATH} = "$ENV{PATH};C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.16299.0\\x64";
		BuildOpenSSL::buildOpenSSL ($openSSL, $arch);
	}
	elsif ($os_name eq 'linux')
	{
		my $openSSL =  "$root/openssl";
		BuildOpenSSL::buildOpenSSL ($openSSL, $arch);
		if ( ! -d "qtbase-${platform}/lib")
		{
			make_path "qtbase-${platform}/lib" or die "mkdir failed: $!";
		}
		copy("openssl/openssl-${platform}/lib/libssl.so.1.0.0", "qtbase-${platform}/lib/libssl.so.1.0.0") or die "Copy failed: $!";
		symlink("libssl.so.1.0.0", "qtbase-${platform}/lib/libssl.so") or die "Symlink failed: $!\n";
		copy("openssl/openssl-${platform}/lib/libcrypto.so.1.0.0", "qtbase-${platform}/lib/libcrypto.so.1.0.0") or die "Copy failed: $!";
		symlink("libcrypto.so.1.0.0", "qtbase-${platform}/lib/libcrypto.so") or die "Symlink failed: $!\n";
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
	my $zipCmd = '"7z"';

	if ($os_name eq 'darwin')
	{
		$zipCmd = '"./BuildTools/MacUtils/7za"';
	}
	elsif ($os_name ne 'linux' && $os_name ne 'MSWin32')
	{
		die ("Unknown platform $os_name");
	}
	doSystemCommand("$zipCmd a -r build/builds.7z ./qtbase-$platform/*");
}

sub patch
{
	        #the default rpath for the xcb plugin is not correct 
                #when distributing the local libs.
		#patch it up with chroot after it's done. (Linux only)
		my ($arch) = @_;
		my $os_name = $^O;
		my $platform = $platforms{$os_name}->{$arch};
		if ($os_name ne 'linux')
		{
		        return;
		}
		my $origin = '$ORIGIN';
		doSystemCommand("chrpath --replace \'$origin/..\' ./qtbase-$platform/plugins/platforms/libqxcb.so");
}

sub main
{
	my %params = getArgs ();
	clean ();
	prepare ($params{arch});
	configure ($params{arch});
	make ($params{arch});
	patch ($params{arch});
	zip ($params{arch});
}

main();
