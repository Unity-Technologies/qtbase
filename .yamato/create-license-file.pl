#!/usr/bin/env perl

# This code is extracted from build.pl

use warnings;
use strict;
use Cwd qw[getcwd];
use File::Copy;
use File::Path qw[make_path];
use File::Spec::Functions qw[canonpath];
use Getopt::Long;
use JSON;
use Params::Check qw[check];

sub createLicenseMd
{
    my $json_file = "LICENSES.json";
    my $json_text = do {
        #open(my $json_fh, "<:encoding(UTF-8)", $json_file)
        open(my $json_fh, "<", $json_file)
            or die("Can't open \"$json_file\": $!\n");
        local $/;
        <$json_fh>
    };

    my $json = JSON->new;
    my $data = $json->decode($json_text);

    open(FH, '>', "LICENSE.md") or die $!;

    print FH "# Licenses which apply to qtbase\n\n";

    for ( @{$data} ) {
        print FH "## " . $_->{Name} . "\n";
        print FH "\n";

        my $relativePath = File::Spec->abs2rel($_->{Path}, getcwd());
        print FH "Path: $relativePath\n\n";

        print FH $_->{Copyright} . "\n\n";

        if (($_->{LicenseId} ne "") && ($_->{LicenseId} ne "NONE"))
        {
            print FH "(" . $_->{LicenseId} . ")\n\n";
        }

        if ($_->{LicenseFile} ne "")
        {
            my $lt = do {
                open(my $lt_fh, "<", $_->{LicenseFile})
                    or die("Can't open \"$_->{LicenseFile}\": $!\n");
                local $/;
                <$lt_fh>
            };

            print FH $lt . "\n\n";
        }
        else
        {
            print FH "\n\n";
        }
    }

    close(FH);
}

sub main
{
	createLicenseMd();
}

main();
