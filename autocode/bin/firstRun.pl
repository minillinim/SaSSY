#!/usr/bin/perl
#
#    File: runFirst.pl
#
#    Copyright (C) 2009 2010 Michael Imelfort, Dominic Eales
#
#    This file is part of the Sassy assembler project
#
#    Sassy is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Sassy is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
#
#    Description:
#    Run this the first time after you use maike.pl
# 
#    Original Authors: Mike Imelfort and Dominic Eales 2009 2010
#

#pragmas
use strict;
use warnings;

#core Perl modules
use Getopt::Long;

BEGIN {
    select(STDERR);
    $| = 1;
    select(STDOUT);
    $| = 1;
}

my $options = check_params();
if(last_chance() == 1) { renameStem(); }
else { print "Aborting\n"; }

sub check_params {
    my @standard_options = ( "help+", "man+", "stem:s", "temp+");
    my %options;

    # Add any other command line options, and the code to handle them
    GetOptions( \%options, @standard_options );

    # If the -help option is set, print the usage and exit
    #
    exec("pod2usage $0") if $options{'help'};

    # If the -man option is set, run perldoc for this
    #
    exec("perldoc $0") if $options{'man'};

    # Check to see that an configuration file has been specified
    #
    if(!($options{'stem'})) {
        print "You need to specify a file\n";
        exec("pod2usage $0");
    }
        
    return \%options;
}


sub last_chance {
    my $files = qq{About to write over: \n};
    if($options->{'temp'})
    {
        $files .= qq{\t$options->{'stem'}MemWrapper.extended.h, $options->{'stem'}MemWrapper.tempConf.h, };
    }
    else
    {
        $files .= qq{\t$options->{'stem'}MemWrapper.cpp, };
    }
    $files .= qq{$options->{'stem'}Class.cpp\n\nAre you sure? [yes/no]:};
    print $files;
    my $answer = <>;
    while(1) {
        chomp $answer;
        if($answer eq "yes" or $answer eq "YES") {return 1;}
        if($answer eq "NO" or $answer eq "no") {return 0;}
        print "Please answer \"yes\" or \"no\":";
        $answer = <>;
    }
}

sub renameStem {
    # first the memWrappers
    if($options->{'temp'})
    {
        my $run_string = qq{mv $options->{'stem'}MemWrapper.extended.h.template $options->{'stem'}MemWrapper.extended.h};
        `$run_string`;
        $run_string = qq{mv $options->{'stem'}MemWrapper.tempConf.h.template $options->{'stem'}MemWrapper.tempConf.h};
        `$run_string`;
    }
    else
    {
        my $run_string = qq{mv $options->{'stem'}MemWrapper.cpp.template $options->{'stem'}MemWrapper.cpp};
        `$run_string`;
    }

    # now do the Classes:
    my $run_string = qq{mv $options->{'stem'}Class.cpp.template $options->{'stem'}Class.cpp};
    `$run_string`;
}

__DATA__

=head1 NAME
   
   runFirst.pl

=head1 COPYRIGHT

   Copyright (c) 2009 2010 Michael Imelfort, Dominic Eales 

=head1 DESCRIPTION

   runFirst.pl

    After calling maike.pl you will have a number of template files. This
    script simply removes the .template from the end of the files
       
=head1 SYNOPSIS

   runFirst.pl [options]


      --help                        Displays basic usage information
      --man                         Displays more detailed information
      --stem                        Main stem of the file to rename
      --temp                        Flag for when the class is a template class

      from the src directory:

      ../autocode/bin/firstRun.pl --stem ___XXX___ [--temp]

      where ___XXX___ is the stem name of a class


=cut