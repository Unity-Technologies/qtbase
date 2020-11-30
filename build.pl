#!/usr/bin/env perl

use lib ".";
use BuildOpenSSL;

use warnings;
use strict;
use Cwd qw[getcwd];
use Digest;
use File::Copy;
use File::Spec::Functions qw[canonpath];
use Getopt::Long;
use JSON;
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
		return ("$launchVisualStudioEnv $platform 10.0.16299.0 && configure -platform win32-msvc -prefix %CD%\\qtbase-$platform -opensource -confirm-license -no-opengl -no-icu -nomake examples -nomake tests -no-dbus -no-harfbuzz -strip -openssl-linked OPENSSL_LIBS=\"-lssleay32 -llibeay32 -lgdi32 -luser32\" -I \"$openSSL\\openssl-$platform\\include\" -L \"$openSSL\\openssl-$platform\\lib\"");
	}
	elsif ($os_name eq 'darwin')
	{
		return ("./configure -platform $platform -prefix `pwd`/qtbase-$platform -opensource -confirm-license -no-icu -nomake examples -nomake tests -no-framework -qt-pcre");
	}
	elsif ($os_name eq 'linux')
	{
		$openSSL = "$root/openssl";
		return ("OPENSSL_LIBDIR='$openSSL/lib' OPENSSL_INCDIR='$openSSL/include' OPENSSL_LIBS='-lssl -lcrypto' ./configure -prefix $root/qtbase-$platform -c++std c++11 -opensource -confirm-license -no-icu -qt-xcb -nomake tests -nomake examples -no-harfbuzz -qt-pcre -qt-libpng -openssl-linked -I $openSSL/openssl-$platform/include -L $openSSL/openssl-$platform/lib");
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
		$zipCmd = '"7za"';
	}
	elsif ($os_name ne 'linux' && $os_name ne 'MSWin32')
	{
		die ("Unknown platform $os_name");
	}
	doSystemCommand("$zipCmd a -r build/builds.7z ./qtbase-$platform/*");

	my $hf = Digest->new("SHA-256");
	$hf->addfile("build/builds.7z");
	my $hash = $hf->hexdigest;
	move("build/builds.7z", "build/5.9.7_$hash.7z");
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

sub createLicenseMd
{
    my $arch = $_[0];
    my $os_name = $^O;
    my $platform = $platforms{$os_name}->{$arch};

    my $sourceArtifact = $_[1];

    my $qtattributionsscanner = "qtattributionsscanner";
	if ($os_name eq 'MSWin32')
    {
        $qtattributionsscanner = "qtattributionsscanner.exe";
    }

    if (-e "/tmp/LICENSES.json.OVERRIDE")
    {
        copy("/tmp/LICENSES.json.OVERRIDE", "LICENSES.json") or die "Copy failed: $!";
    }
    else
    {
        system("$qtattributionsscanner --output-format json -o LICENSES.json .") == 0
            or die("failed to run qtattributionsscanner: $?");
    }

    my $json_file = "LICENSES.json";
    my $json_text = do {
        open(my $json_fh, "<:encoding(UTF-8)", $json_file)
            or die("Can't open \"$json_file\": $!\n");
        local $/;
        <$json_fh>
    };

    my $json = JSON->new;
    my $data = $json->decode($json_text);

    open(FH, '>', "./qtbase-$platform/LICENSE.md") or die $!;

    if ($sourceArtifact ne "")
    {
        print FH "# Source code\n\n";
        print FH "The corresponding source code can be found at:\n";
        print FH "https://public.stevedore.unity3d.com/r/public/$sourceArtifact\n\n";
    }

    print FH "# Licenses\n\n";

    for ( @{$data} ) {
        print FH "## " . $_->{Name} . "\n";
        print FH "\n";

        my $relativePath = File::Spec->abs2rel($_->{Path}, getcwd());
        print FH "Path: $relativePath\n\n";

        print FH $_->{Copyright} . "\n\n";

        if ($_->{LicenseId} ne "")
        {
            print FH "(" . $_->{LicenseId} . ")\n\n";
        }

        my $lt = do {
            open(my $lt_fh, "<:encoding(UTF-8)", $_->{LicenseFile})
                or die("Can't open \"$_->{LicenseFile}\": $!\n");
            local $/;
            <$lt_fh>
        };

        print FH $lt . "\n\n";
    }

    close(FH);
}

sub getSourceArtifact
{
    my $dir = "qtbase-src";
    if ( ! -d $dir)
    {
        # Don't break local builds if the file is not available.
        return "";
    }

    opendir (DIR, $dir) or die $!;

    while (my $file = readdir(DIR)) {
        if ($file =~ m/\.zip$/)
        {
            return "$dir/$file";
        }
    }

    die "qtbase-src dir exists, but does not contain any zip files :(";
}

sub main
{
	my %params = getArgs ();
	my $sourceArtifact = getSourceArtifact ();
	clean ();
	prepare ($params{arch});
	configure ($params{arch});
	make ($params{arch});
	patch ($params{arch});
	createLicenseMd ($params{arch}, $sourceArtifact);
	zip ($params{arch});
}

main();
