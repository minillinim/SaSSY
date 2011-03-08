#!/usr/bin/perl
#
#    File: maike.pl
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
#
#    this file reads from the DataManagerDefs.conf file
#    and produces the DataManagerDefs.cpp and DataManagerDefs.h files
#    for use with the DataManager object found in DataManager.cpp/.h
#    You will only need to call this script when you update the
#    DataManagerDefs.conf file
# 
#    Original Authors: Mike Imelfort and Dominic Eales 2009 2010 2011
#

#pragmas
use strict;
use warnings;
use POSIX qw(ceil floor);

#core Perl modules
use Getopt::Long;
use Data::Dumper;
use String::CamelCase qw(camelize);
use Class::Struct;

# copyright info
my $copy_right_years = "2009 2010 2011";
my $copy_right_holders = "Michael Imelfort and Dominic Eales";

#CPAN modules

#locally-written modules

BEGIN {
    select(STDERR);
    $| = 1;
    select(STDOUT);
    $| = 1;
}

# Class specific fields, MasterClasses and SubClasses are represented as structs
# we define them here!
# Classes can be master classes or sub classes. Some objects will need both and some only one
# when an object needs only one then the sub_class name matches the master_class name
# we only write .cpp .h and .make files for master classes
# sub classes affect the nuimber and type of #define statements in a master class file
# and determine the get and set methods

# for objects
struct Field =>
{
    name => '$',                # name of the field
    classID => '$',             # which class does this field point to (if any)
    systemType => '$',          # system defined type of the field
    size => '$',                # size of the field in bits
    comment => '$',             # comments
    mask => '$',                # bitmask hex string
    anti_mask => '$',           # the opposite of the mask
    offset => '$',              # offset of the int containing this field in the block
    shifter => '$',             # offset of this field in it's containing int
    c_inc => '$',               # for signed and unsigned counters (this increments)
    c_sign => '$',              # used only for signed counters
};

struct SubClass =>
{
    name => '$',                # subClass name
    classID => '$',             # user defined type of this subClass
    prefix => '$',              # prefix applied t hash defines
    fields => '@',              # array of field objects    ORDER MATTERS!!!
    numFields => '$',           # the number of fields
    comment => '$',             # comments
};

struct MasterClass =>
{
    name => '$',                # the name of the masterClass
    subClasses => '%',          # hash of subClasses
    autoCode => '$',            # auto code the set get methods?
    numSubClasses => '$',       # the number of subClasses
    reSizing => '@',            # the resizing data for the master class
    deleteable => '$',          # is the memManager deleteable?
    hashDefines => '@',         # list of hash defines to be written into the file
    
    #
    # For templated objects
    #
    isTemplated => '$',         # is this a template class?
    templateParams => '@',      # array of template parameters
    blockSizeName => '$',       # the name of the BLOCK_SIZE # define
};

# plus template objects specials

struct TemplateParam =>
{
    name => '$',                # the name of the template parameter as it should be written
    type => '$',                # the C++ type as it shoule be written
    default => '$',             # the default value
    comment => '$',             # descriptor
};


# the syntax of the .conf file:
# we can store this in a hash to make life a little easier
    my %file_offsets = ();
 
# Syntax for the global definitions
# [:Name:Value:Comment
    $file_offsets{'G_Name'} = 1;
    $file_offsets{'G_Value'} = 2;
    $file_offsets{'G_Com'} = 3;

# Syntax for the Class definitions
# ]:MasterClassName:SubClassName:templated?:ClassID:Prefix:DeleteableMemManager:Autocode?:firstDivisor[,nextDivisor,...]:Comment
    $file_offsets{'C_MasterClassName'} = 1;
    $file_offsets{'C_SubClassName'} = 2;
    $file_offsets{'C_Templated'} = 3;
    $file_offsets{'C_ID'} = 4;
    $file_offsets{'C_Prefix'} = 5;
    $file_offsets{'C_Del'} = 6;
    $file_offsets{'C_AutoCode'} = 7;
    $file_offsets{'C_ReSizes'} = 8;
    $file_offsets{'C_Com'} = 9;
    
# Syntax for class specific field definitions:
# |:SubClassName:
    $file_offsets{'S_SubClassName'} = 1;
# D:HashDefineName:HashDefineValue
    $file_offsets{'S_Define_Name'} = 1;
    $file_offsets{'S_Define_Value'} = 2;
# T:TemplateParamName:TemplateParamType:TemplateParamComment
    $file_offsets{'S_Template_Name'} = 1;
    $file_offsets{'S_Template_Type'} = 2;
    $file_offsets{'S_Template_Default'} = 3;
    $file_offsets{'S_Template_Comment'} = 4;
# B:Blocksize (for a #define)
    $file_offsets{'S_BlockSize'} = 1;
    
# ~:Field:Class_ID:SystemDefinedType:SizeInBits:Comment
    $file_offsets{'F_FieldName'} = 1;
    $file_offsets{'F_ID'} = 2;
    $file_offsets{'F_SysType'} = 3;
    $file_offsets{'F_Size'} = 4;
    $file_offsets{'F_Com'} = 5;

# for handling ID Types
    my %id_hash = ();                       # = { ID : subClassType }

# housekeeping
    my %global_vars = ();                   # other vars passed in via confG
    my @global_defines = ();                # global vars all #defined up!
    my $size_of_int = -1;                   # the number of bits in an int ( compile time derived )
    my $size_of_id_type = -1;               # the number of bits in an ID_TYPE ( hardcoded )
    my $line_counter = 0;                   # line counter of the .conf file for error logging
    my $total_number_of_sub_classes = 0;    # how many sub classes have we defined?
    my $total_number_of_master_classes = 0; # how many master classes only have we defined?
    my @all_files_written = ();             # a list of the files this script has created

# data
    my %masterClasses = ();                 # Hash of masterClass structs = { masterClassName : masterClass }
    my %classIDs = ();                      # link between a specific ClassID and a subClass name = { classID : subClassName }
    my %subClasses = ();                    # for duplicate checking = {subClassName : masterClassName }
    my %prefixes = ();                      # for duplicate checking = { Prefix : subClassName }

# output
    my %can_autocode = ();                  # should we autocode?
    my $layer_1_suffix = "MemManager";      # if master class name = Bob => BobMemManager.cpp and BobMemManager.h; Layer 1 file names
    my $layer_2_suffix = "MemWrapper";      # layer 2 file names
    my $layer_3_suffix = "Class";           # layer 3 file names

# get the user options
my $options = check_params();

print "\nWelcome to maike version 0.1\nCopyright (c) Mike Imelfort and Dominic Eales 2009\n\n" if (!$options->{'silent'});

# parse the conf file into memory
parse_conf();

# pack all the data, make all the masks and offsets
pack_data();

# do sum autocoding!!
auto_code_all();

#########################################################################
# SUBS
#########################################################################
#
# Check parameters and call appropriate routines if necessary
#
sub check_params {
    my @standard_options = ( "help+", "man+", "temps:s", "confG:s", "confO:s", "silent+");
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
    if(!($options{'temps'})) {
        $options{'temps'} = "../autocode/templates/";
    }

    # Check to see that an configuration file has been specified
    #
    if(!($options{'confG'})) {
        $options{'confG'} = "GlobalDefs.conf"; 
    }
    
    # Check to see that an configuration file has been specified
    #
    if(!($options{'confO'})) {
        print "No objects to build, slacking off now!\n";
        exec("pod2usage $0");
    }
    
    return \%options;
}

# 
# Parse the conf file into memory
#
sub parse_conf {

    # first try to load the configuration files
    my @all_confs;
    my $CONF_FILE;

    # the global file
    print qq{loading configuration files from $options->{'temps'}\n}  if (!$options->{'silent'});
    print qq{loading global configuration file: $options->{'confG'}\n}  if (!$options->{'silent'});
    open($CONF_FILE, "<".$options->{'temps'}.$options->{'confG'}) || die qq{Can't open global configuration file [$options->{'temps'}$options->{'confG'}]: $!.\n};
    while (<$CONF_FILE>) {
        next if $_ =~ "^#" || $_ =~ "^\n";
        chomp $_;
        push @all_confs, $_;
    }
    close $CONF_FILE;

    # individual conf files
    my @conf_files = split /,/, $options->{'confO'};
    foreach my $conf_file_name (@conf_files)
    {
        print qq{loading object configuration file: $conf_file_name \n} if (!$options->{'silent'});
        open($CONF_FILE, qq{<$options->{'temps'}$conf_file_name}) || die qq{Can't open configuration file [$options->{'temps'}$conf_file_name]: $!.\n};
        while (<$CONF_FILE>) {
            next if $_ =~ "^#" || $_ =~ "^\n";
            chomp $_;
            push @all_confs, $_;
        }
        close $CONF_FILE;
    }
    
    # housekeeping, we need to hold these vars until we know what the haps is
    # these guys relate to the structs defined above
    my $current_mc_name = "none";
    my $new_mc;
    my $current_sc;
    my $current_mc;
    
    foreach(@all_confs)
    {
        $line_counter++;
        my @line = split /:/, $_;
        if($line[0] eq '[') {
            # Global definition
            if($line[$file_offsets{'G_Name'}] eq 'SIZE_OF_INT')
            {
                $size_of_int = $line[$file_offsets{'G_Value'}] - 0;
            }
            if($line[$file_offsets{'G_Name'}] eq 'SIZE_OF_IDTYPE')
            {
                $size_of_id_type = $line[$file_offsets{'G_Value'}] - 0;
            }   
            $global_vars{$line[$file_offsets{'G_Name'}]} = $line[$file_offsets{'G_Value'}];

            # REMOVED THIS LINE, SHOULDN'T BE NEEDED
            #push @global_defines, qq{#ifndef $line[$file_offsets{'G_Name'}]\n#define $line[$file_offsets{'G_Name'}] $line[$file_offsets{'G_Value'}]\n#endif //$line[$file_offsets{'G_Name'}]};
            
        } elsif($line[0] eq ']') {
            # Class definitions
            # set the current Class and subClass
            if($current_mc_name ne $line[$file_offsets{'C_MasterClassName'}])
            {
                $current_mc_name = $line[$file_offsets{'C_MasterClassName'}];
                if(exists $masterClasses{$current_mc_name})
                {
                    die qq{Error: redefinition of master class $current_mc_name at line: $line_counter\n};
                }
                $new_mc = MasterClass->new();
                $new_mc->name( $line[$file_offsets{'C_MasterClassName'}] );
                $new_mc->numSubClasses( 0 );
                $new_mc->isTemplated( int($line[$file_offsets{'C_Templated'}]) );
                # user definesd autocode flag. We need store this now
                # and will update autocode if there is a conf file supplied
                if(int($line[$file_offsets{'C_AutoCode'}]) == 1) {
                    $can_autocode{$new_mc->name} = 1;
                }
                $new_mc->autoCode( 0 );
                $new_mc->deleteable( int($line[$file_offsets{'C_Del'}]) );
                $masterClasses{$current_mc_name} = [ $new_mc ];
            }
            # add the subclass provided that the subClass name is diffeent from
            # the masterClass name

            # work out the proper ID type for this subclass now
            $id_hash{$line[$file_offsets{'C_ID'}]} = $line[$file_offsets{'C_SubClassName'}]."Id";

            # check if the subClass name, Id and Prefix are unique
            my $tmp_string;
            if(exists $subClasses{$line[$file_offsets{'C_SubClassName'}]})
            {
                $tmp_string = $line[$file_offsets{'C_SubClassName'}];
                die qq{Error: redefinition of sub class $tmp_string at line: $line_counter\n};
            }
            else
            {
                 $subClasses{$line[$file_offsets{'C_SubClassName'}]} = $new_mc->name;
            }
            if(exists $classIDs{$line[$file_offsets{'C_ID'}]})
            {
                $tmp_string = $line[$file_offsets{'C_ID'}];
                die qq{Error: redefinition of class ID $tmp_string at line: $line_counter\n};
            }
            else
            {
                $classIDs{$line[$file_offsets{'C_ID'}]} = $line[$file_offsets{'C_SubClassName'}];
            }
            if(exists $prefixes{$line[$file_offsets{'C_Prefix'}]})
            {
                $tmp_string = $line[$file_offsets{'C_Prefix'}];
                die qq{Error: redefinition of class prefix $tmp_string at line: $line_counter\n};
            }
            else
            {
                $prefixes{$line[$file_offsets{'C_Prefix'}]} = $line[$file_offsets{'C_SubClassName'}];
            }

            # we need only worry about making the subclass when we need to autocide it
            if(0 != int($line[$file_offsets{'C_AutoCode'}]))
            {
                my $new_sub_class = SubClass->new();
                $new_sub_class->name( $line[$file_offsets{'C_SubClassName'}] );
                $new_sub_class->classID( $line[$file_offsets{'C_ID'}] );
                $new_sub_class->prefix( $line[$file_offsets{'C_Prefix'}] );
                $new_sub_class->numFields( 0 );
                $new_sub_class->comment( $line[$file_offsets{'C_Com'}] );
                # if the subclass name is the same as the masterclass name AND autocode == 1
                # then the resizing varibles will be used.
                # resizing looks like this: x[,y,z,...,l]
                # the original number of containers to be allocated 'O'is given at runtime.
                # When this is full the memManager will allocate another O/x containers and
                # then another O/y when these are full and so on
                # the last divisor 'l' is stored and all subsequent allocations are of size O/l
                # you MUST supply at least one divisor
                if($new_mc->name eq $new_sub_class->name && exists $can_autocode{$new_mc->name})
                {
                    $new_mc->blockSizeName( $new_sub_class->prefix."BLOCK_SIZE" );
                    my @re_sizes = split /,/, $line[$file_offsets{'C_ReSizes'}];
                    foreach my $size_var (@re_sizes)
                    {
                        push @{$new_mc->reSizing}, $size_var;
                    }
                }
                # add this sub class to the existing_mc
                $new_mc->subClasses( $new_sub_class->name  => [ $new_sub_class ] );
                $new_mc->numSubClasses($new_mc->numSubClasses + 1);
            }
        } elsif($line[0] eq '|') {
            # set the current subClass
            my $mc_reference = $masterClasses{$subClasses{$line[$file_offsets{'S_SubClassName'}]}};
            $current_mc = @{$mc_reference}[0];
            my $sc_reference = $current_mc->subClasses($line[$file_offsets{'S_SubClassName'}]);
            $current_sc = @{$sc_reference}[0];
            if($current_mc->name eq $current_sc->name && exists $can_autocode{$current_mc->name})
            {
                # set the autocode var now
                $current_mc->autoCode( 1 );
            }
        } elsif($line[0] eq 'T') {
            # template definiton
            if($current_mc->isTemplated != 1)
            {
                die("Template vars defined for non-template master class: ".$current_mc->name."\n");
            }
            my $new_template_param = TemplateParam->new();
            $new_template_param->name( $line[$file_offsets{'S_Template_Name'}] );
            $new_template_param->type( $line[$file_offsets{'S_Template_Type'}] );
            $new_template_param->default( $line[$file_offsets{'S_Template_Default'}] );
            $new_template_param->comment( $line[$file_offsets{'S_Template_Comment'}] );
            push @{$current_mc->templateParams}, [$new_template_param];
            
        } elsif($line[0] eq 'B') {
            # blocksize rule
            push @{$current_mc->hashDefines}, "#define ".$current_mc->blockSizeName." ".$line[$file_offsets{'S_BlockSize'}];

        } elsif($line[0] eq 'D') {
            # #define for this guy
            push @{$current_mc->hashDefines}, qq{#define $line[$file_offsets{'S_Define_Name'}] $line[$file_offsets{'S_Define_Value'}]};
            
        } elsif($line[0] eq '~') {
            my $new_field = Field->new();
            $new_field->name( $line[$file_offsets{'F_FieldName'}] );
            $new_field->classID( $line[$file_offsets{'F_ID'}] );
            $new_field->systemType( $line[$file_offsets{'F_SysType'}] );
            $new_field->size( $line[$file_offsets{'F_Size'}] );
            $new_field->comment( $line[$file_offsets{'F_Com'}] );
            # add this field to the current subclass          
            push @{$current_sc->fields}, [$new_field];
            $current_sc->numFields($current_sc->numFields + 1);
        } else {
            # Crud!
            die "Error: Incompatible sytax: $line_counter: \"".$line[0]."\" : \"$_\"\n";
        }        
    }

    if($size_of_int < 0)
    {
        die "Error: You need to set the size of integers in bits\n";
    }
    print "\nConfiguration parsing seems to have worked\n" if (!$options->{'silent'});
}

#
# pack all the data and make the masks and the offsets
#
sub pack_data {
    # pointers and variable data take up sizes set in GlobalDefs.conf
    my($data_mask, $data_anti_mask, $data_bit_shift) = create_masks(0, $size_of_int);
    while (( my $mc_name, my $mc_ref) = each(%masterClasses))
    {
        # get the master class and check it has subclasses
        my $current_mc = @{$mc_ref}[0];
        if($current_mc->numSubClasses > 0)
        {
            my %sc_hash = %{$current_mc->subClasses};
            while((my $sc_name, my $sc_ref ) = each(%sc_hash) )
            {
                my $current_sc = @{$sc_ref}[0];
                if($current_sc->numFields > 0)
                {
                    my $prev_offset_define = "";
                    my $current_offset_define = $current_sc->prefix."BASE_OFFSET";
                    my $base_offset_define = $current_offset_define;
                    my $next_field_jump = 0;
                    push @{$current_mc->hashDefines}, "\n// offsets for the sublass:\n#define $current_offset_define 0";
                    
                    my $bit_counter = 0;
                    my @field_array = @{$current_sc->fields};
                    
                    foreach my $field_ref (@field_array)
                    {
                        my $current_field = @{$field_ref}[0];
                        
                        $prev_offset_define = $current_offset_define;
                        $current_offset_define = $current_sc->prefix.$current_field->name."_OFFSET";
                        
                        if($current_field->systemType eq "MMG_POINTER") {
                            if($bit_counter + $size_of_id_type > $size_of_int)
                            {
                                $bit_counter = 0;
                                $next_field_jump++;
                            }

                            my($point_mask, $point_anti_mask, $point_bit_shift) = create_masks($bit_counter, $size_of_id_type);

                            $current_field->mask($point_mask);
                            $current_field->anti_mask($point_anti_mask);
                            $current_field->shifter($point_bit_shift);
                            $current_field->offset($current_offset_define);
                            push @{$current_mc->hashDefines}, "#define $current_offset_define ($base_offset_define + $next_field_jump)";
#                             push @{$current_mc->hashDefines}, "#define $current_offset_define ($prev_offset_define + $next_field_jump)";
                            $bit_counter += $size_of_id_type;
                            if($bit_counter >= $size_of_int)
                            {   
                                $bit_counter = 0;
                                $next_field_jump++;
                            }
                            
                        } elsif($current_field->systemType eq "MMG_VAR_DATA") {
                            $current_field->mask($data_mask);
                            $current_field->anti_mask($data_anti_mask);
                            $current_field->shifter($data_bit_shift);
                            $current_field->offset($current_offset_define);
                            push @{$current_mc->hashDefines}, "#define $current_offset_define ($base_offset_define + $next_field_jump)";
#                             push @{$current_mc->hashDefines}, "#define $current_offset_define ($prev_offset_define + $next_field_jump)";
                            $next_field_jump = $current_field->size;
                            $bit_counter = 0;
                        } else {
                            if($bit_counter + $current_field->size > $size_of_int)
                            {
                                $bit_counter = 0;
                                $next_field_jump++;
                            }
                            my($mask, $anti_mask, $bit_shift) = create_masks($bit_counter, $current_field->size);
                            $current_field->mask($mask);
                            $current_field->anti_mask($anti_mask);
                            $current_field->offset($current_offset_define);
                            $current_field->shifter($bit_shift);
                            my $counter_inc;
                            my $counter_sign;
                            my $signed_field_mask;
                            my $signed_field_anti_mask;
                            if($current_field->systemType eq "MMG_S_COUNTER" or $current_field->systemType eq "MMG_COUNTER")
                            {
                                ($counter_inc,$counter_sign,$signed_field_mask,$signed_field_anti_mask) = create_counter_masks($bit_counter, $current_field->size);
                                $current_field->c_inc($counter_inc);
                                if($current_field->systemType eq "MMG_S_COUNTER")
                                {
                                    $current_field->c_sign($counter_sign);
                                    $current_field->mask($signed_field_mask);
                                    $current_field->anti_mask($signed_field_anti_mask);
                                }
                            }
                            push @{$current_mc->hashDefines}, "#define $current_offset_define ($base_offset_define + $next_field_jump)";
#                             push @{$current_mc->hashDefines}, "#define $current_offset_define ($prev_offset_define + $next_field_jump)";
#$next_field_jump = 0;
                            $bit_counter += $current_field->size;
                            if($bit_counter >= $size_of_int)
                            {   
                                $bit_counter = 0;
                                $next_field_jump++;
                            }
                        }
                    }
                }
            }
        }
    }
}

#
# make a mask, given the start point and the length in bits
# start is zero indexed
#
sub create_masks {
    my ($start, $length) = @_;
    my $tmp_nb = $size_of_int;
    my @masks = ("", "");

    while($start > 0)
    {
        $start--;
        $tmp_nb--;
        $masks[0] .= "0";
        $masks[1] .= "1";
    }
    while($length > 0)
    {
        $length--;
        $tmp_nb--;
        $masks[0] .= "1";
        $masks[1] .= "0";
    }

    $masks[2] = $tmp_nb;
    
    while($tmp_nb > 0)
    {
        $tmp_nb--;
        $masks[0] .= "0";
        $masks[1] .= "1";
    }

    $masks[0] = "0x".uc(unpack("H*", pack ("B*", $masks[0])));
    $masks[1] = "0x".uc(unpack("H*", pack ("B*", $masks[1])));
    return @masks;
}

# 
# Make masks for counters
# We need an increment and decrement mask and for signed counters
# we need a signed bit
# 
sub create_counter_masks {
    my ($start, $length) = @_;
    my $tmp_nb = $size_of_int;
    my $counter_inc = "";
    my $counter_sign = "";
    my $signed_field_mask = "";
    my $signed_field_anti_mask = "";
    while($start > 0)
    {
        $start--;
        $tmp_nb--;
        $counter_inc .= "0";
        $counter_sign .= "0";
        $signed_field_mask .= "0";
        $signed_field_anti_mask .= "1";
    }
    my $first_run = 1;
    while($length > 0)
    {
        if($first_run == 1) {
            $first_run = 0;
            $counter_sign .= "1";
            $signed_field_mask .= "0";
        } else {
            $counter_sign .= "0";
            $signed_field_mask .= "1";
        }
        if($length == 1) {
            $counter_inc .= "1";
        } else {
            $counter_inc .= "0";
        }
        $signed_field_anti_mask .= "0";
        $length--;
        $tmp_nb--;
    }
    
    while($tmp_nb > 0)
    {
        $tmp_nb--;
        $counter_inc .= "0";
        $counter_sign .= "0";
        $signed_field_mask .= "0";
        $signed_field_anti_mask .= "1";
    }
    $counter_inc = "0x".uc(unpack("H*", pack ("B*", $counter_inc)));
    $counter_sign = "0x".uc(unpack("H*", pack ("B*", $counter_sign)));
    $signed_field_mask = "0x".uc(unpack("H*", pack ("B*", $signed_field_mask)));
    $signed_field_anti_mask = "0x".uc(unpack("H*", pack ("B*", $signed_field_anti_mask)));
    return ($counter_inc, $counter_sign, $signed_field_mask, $signed_field_anti_mask);
}

#
# logs of all flavours!!!
#
sub log_base {
    my ($base, $value) = @_;
    return log($value)/log($base);
}

#
# Do sum autocoding
#
sub auto_code_all
{
    auto_code_MemManager();
    auto_code_Wrapper();
    auto_code_Class();
    if (!$options->{'silent'}) {
        print "\nCreated files:\n";
        foreach my $file (@all_files_written) {
            print "\t$file\n";
        }       
    }
}

#
# Automatically code all the get/set/etc.. methods for the MemManager
#
sub auto_code_MemManager {
    my $OUT_CPP;                                                    # .cpp file
    my $OUT_H;                                                      # .h file
    my $tab = "    ";                                               # tab character
    
    while (( my $mc_name, my $mc_ref) = each(%masterClasses))
    {
        # get the master class and check it needs autocoding
        my $current_mc = @{$mc_ref}[0];
        if(1 == $current_mc->autoCode)
        {
            # try to open the files for writing. We need a .cpp a .h and a makefile.
            # work out the correct file location relative to the project root
            my $cpp_file = $current_mc->name.$layer_1_suffix.".cpp";
            my $h_file = $current_mc->name.$layer_1_suffix.".h";
            open ($OUT_CPP, ">$cpp_file") || die "Can't write to file [$cpp_file]: $!.\n";
            open ($OUT_H, ">$h_file") || die "Can't write to file [$h_file]: $!.\n";
            push @all_files_written, $cpp_file;
            push @all_files_written, $h_file;
            my $mem_manager_name = $current_mc->name."MemManager";
            my $h_include = "";
            if($current_mc->isTemplated != 1)
            {
                $h_include = "\n#include \"$h_file\"\n";
            }

print $OUT_CPP <<"EOF";
/******************************************************************************
**
** File: $cpp_file
**
*******************************************************************************
**
** Layer 1 Implementation file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It includes implementations to get/set all the fields defined in the
** .conf file supplied as an argument to maike.pl
**
** DO NOT EDIT THIS FILE
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

// system includes
#include <iostream>

// local includes
#include "MemManager.h"
#include "intdef.h"
#include "math_ext.h"
#include "IdTypeDefs.h"$h_include

using namespace std;

/******************************************************************************
 ** Implementation of $mem_manager_name functions declared inline in $h_file
******************************************************************************/

EOF
            my $if_def_string = $mc_name."_MemManager_h";
            if($current_mc->isTemplated == 1)
            {
                print $OUT_CPP "#ifdef $if_def_string\n\n";
            }

print $OUT_H <<"EOF";
/******************************************************************************
**
** File: $h_file
**
*******************************************************************************
**
** Layer 1 Header file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It includes implementations to get/set all the fields defined in the
** .conf file supplied as an argument to maike.pl
**
** DO NOT EDIT THIS FILE
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

#ifndef $if_def_string
    #define $if_def_string

// system includes 
#include <iostream>
#include <vector>

// local includes
#include "MemManager.h"
#include "intdef.h"
#include "IdTypeDefs.h"

using namespace std;

EOF

# print local hash defines
foreach my $defined (@global_defines)
{
    print $OUT_H $defined."\n";
}

# print local hash defines
my @hash_defines = @{$current_mc->hashDefines};
foreach my $defined (@hash_defines)
{
    print $OUT_H $defined."\n";
}

# The main mem manager...
            my $block_size = $current_mc->blockSizeName;
            my $MM_TYPE = "";
            if(1 == $current_mc->deleteable)
            {
                $MM_TYPE = "DeletableMemManager";
            }
            else
            {
                $MM_TYPE = "MemManager";
            }

            # for templated objects we need to work out what the template string will look like
            my $longTemplateString = " ";
            my $shortTemplateString = "";
            if($current_mc->isTemplated == 1)
            {
                $longTemplateString = "template <";
                $shortTemplateString = "<";
                my $first_run = 1;
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    if($first_run == 1) { $first_run = 0; }
                    elsif($first_run == 0) { $longTemplateString .= ", "; $shortTemplateString .= ", "; }
                    my $current_tp = @{$tp_ref}[0];
                    $longTemplateString .= $current_tp->type." ".$current_tp->name;
                    $shortTemplateString .= $current_tp->name;
                }
                $longTemplateString .=">";
                $shortTemplateString .=">";
            }
            
print $OUT_H <<"EOF";

/******************************************************************************
 ** Implementation of $mc_name MemManager wrapper
******************************************************************************/

$longTemplateString class $mem_manager_name {
    public:
  
  // Constructor/Destructor

    // trivial construtor
    $mem_manager_name() { mData = NULL; }
        
    // all we need to do is delete the memManager
    ~$mem_manager_name(void) { if(mData != NULL) { delete mData; } }


    bool initialise(idInt originalNumberOfBlocks) {
        //-----
        // Initialise this guy. Mostly a wrapper for the basic MemManager
        //
        vector<idInt> temp;
EOF

            my @sizes_array = @{$current_mc->reSizing};
            for(my $i = 0; $i <= $#sizes_array; $i++)
            {
                print $OUT_H $tab.$tab."temp.push_back($sizes_array[$i]);\n";
            }
print $OUT_H <<"EOF";

        mData = new $MM_TYPE<$block_size>;
        PARANOID_ASSERT_L1(mData != NULL);

        // set the name
        mData->_name="$mem_manager_name";

        // initialisation of the base MemManager
        if(!mData->initialise(originalNumberOfBlocks, temp))
            return false;
        temp.clear();

        #ifdef MAKE_PARANOID
            mData->debugvars();
        #endif

        return true;
    }

    //
    // saving and loading
    //
    bool save( string fileName ) { return mData->save( fileName + ".base_mm" ); }
    
    bool initialise( string fileName ) {
        mData = new $MM_TYPE<$block_size>;
        PARANOID_ASSERT_L1(mData != NULL);

        // set the name
        mData->_name="$mem_manager_name";
        
        return mData->initialise( fileName  + ".base_mm");
    }
    
    //
    // This section is used to get to the inner workings of the ID type
    // for this reason you should treat these functions with care. 
    // Wrap and unWrap should not see the light of day.
    //
    
    template<class T>
    inline idInt unWrapPointer(T ID)
    {
        //-----
        // Get the guts of the ID
        //
        return mData->unWrapId(ID);
    }

    template<class T>
    inline void wrapPointer(idInt address, T * ID)
    {
        //-----
        // Wrap the value in address in an Id of the right type
        //
        mData->wrapId(address, ID);
    }

EOF

            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    my $create_function_name = "create".$this_ID_type;
                    my $destroy_function_name = "destroy".$this_ID_type;
print $OUT_H <<"EOF";

    inline bool is$this_ID_type(idInt address, $this_ID_type ID)
    {
        //-----
        // see if the idInt given matches the $this_ID_type given
        //
        return (unWrapPointer(ID) == address);
    }

    inline $this_ID_type get$this_ID_type(idInt address)
    {
        //-----
        // get a $this_ID_type Id by it's address
        //
        $this_ID_type ret_id;
        wrapPointer(address, &ret_id);
        return ret_id;
    }

    inline bool isValidAddress($this_ID_type ID)
    {
        //-----
        // Determine if the addres is valid
        //
        return mData->isValidAddress(ID.get());
    }

    inline uMDInt * getAddr($this_ID_type ID)
    {
        //-----
        // get the memory address for this $this_ID_type
        //
        PARANOID_ASSERT_L1(mData->getAddress(ID.get()) != MM_ADDR_NULL);
        return mData->getAddress(ID.get());
    }

    inline $this_ID_type $create_function_name(void)
    {
        //-----
        // get a new $this_ID_type from the MemManager
        //
        $this_ID_type return_ID;
        return_ID.set(mData->getNewId());
        return return_ID;
    }
EOF

                    if(1 == $current_mc->deleteable)
                    {
print $OUT_H <<"EOF";

    inline bool $destroy_function_name($this_ID_type ID)
    {
        //-----
        // delete a $this_ID_type
        //
        return mData->freeId(ID.get());
    }

EOF


                    }
                }
            }
            
print $OUT_H <<"EOF";

    private:

    $MM_TYPE<$block_size> * mData;
};
    
EOF
            # coding functions
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    if($current_sc->numFields > 0)
                    {
                        my $this_ID_type = $id_hash{$current_sc->classID};           # type of this subClass

# New/Free wrappers and Class declaration

print $OUT_CPP <<"EOF";
/******************************************************************************
 ** Implementation of $sc_name L1 Class
******************************************************************************/
EOF

print $OUT_H <<"EOF";

/******************************************************************************
 ** Headers for $sc_name L1 Class
******************************************************************************/

$longTemplateString class $sc_name {
    public:
  
  // Constructor/Destructor
    $sc_name($mem_manager_name$shortTemplateString * dataStore) { mData = dataStore; }
    ~$sc_name(void) {}

  // New/Free wrappers
    inline $this_ID_type new$this_ID_type(void) { return mData->create$this_ID_type(); }
EOF

                        if(1 == $current_mc->deleteable)
                        {
                            print $OUT_H $tab."inline bool delete$this_ID_type( $this_ID_type ID ) { return mData->destroy$this_ID_type( ID ); }\n"
                        }

# Get/Set/Etc.. methods

print $OUT_CPP "\n  // Get/Set/etc.. methods\n";
print $OUT_H "\n  // Get/Set/etc.. methods\n";

                        my @field_array = @{$current_sc->fields};
                        foreach my $field_ref (@field_array)
                        {
                            my $current_field = @{$field_ref}[0];
                            # put the field name in camel case
                            my $function_base_name = camelize(lc($current_field->name));    # suffix for all functions
                            my $function_print_name;                                        # actual print name
                            my $function_comment =  $current_field->comment;                # comment for the function
                            my $current_field_offset = $current_field->offset;              # offset in the block
                            my $current_field_shifter = $current_field->shifter;            # bitshift amount for the field
                            my $current_field_mask = $current_field->mask;                  # bitmask for the field
                            my $current_field_antimask = $current_field->anti_mask;         # inverted bit mask
                            my $input_var =  lc($current_field->name);                      # name of any input var
                            my $IO_ID_type = $id_hash{$current_field->classID};             # type of any input or output var

                            
                            if($current_field->systemType eq "MMG_FLAG")
                            {                                

#--------------------------------------------------------------------------------------------------------
# isFLAG
                                $function_print_name = "is".$function_base_name;
                                print $OUT_H $tab."bool $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    bool $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        return (*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask);
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#setFLAG
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."void $function_print_name(bool $input_var, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(bool $input_var, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        if($input_var)
        {
            *(mData->getAddr(ID) + $current_field_offset) |= $current_field_mask;
        }
        else
        {
            *(mData->getAddr(ID) + $current_field_offset) &= $current_field_antimask;
        }
    }

EOF

#--------------------------------------------------------------------------------------------------------
#clearFLAG
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        *(mData->getAddr(ID) + $current_field_offset) &= $current_field_antimask;
    }
    
EOF

                            }
                            if($current_field->systemType eq "MMG_POINTER")
                            {

#--------------------------------------------------------------------------------------------------------
#getPOINTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."$IO_ID_type $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    $IO_ID_type $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        $IO_ID_type return_ID;
        mData->wrapPointer( (idInt)( (*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) >> $current_field_shifter), &return_ID);
        return return_ID;
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#setPOINTER
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($IO_ID_type $input_var, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($IO_ID_type $input_var, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // clear first
        *(mData->getAddr(ID) + $current_field_offset) &= $current_field_antimask;

        // then set
        *(mData->getAddr(ID) + $current_field_offset) |= ( ( ((uMDInt)(mData->unWrapPointer($input_var))) << $current_field_shifter ) & $current_field_mask );
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#clearPOINTER
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        *(mData->getAddr(ID) + $current_field_offset) &= $current_field_antimask;
    }
    
EOF

                            }
                            if($current_field->systemType eq "MMG_DATA")
                            {

#--------------------------------------------------------------------------------------------------------
#getDATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."uMDInt $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    uMDInt $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        return ((*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) >> $current_field_shifter);
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#setDATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."void $function_print_name(uMDInt data, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(uMDInt data, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // clear first
        (*(mData->getAddr(ID) + $current_field_offset)) &= $current_field_antimask;
        
        // then set
        (*(mData->getAddr(ID) + $current_field_offset)) |= ((data << $current_field_shifter) &  $current_field_mask);
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#clearDATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        (*(mData->getAddr(ID) + $current_field_offset)) &= $current_field_antimask;
    }
    
EOF

                            }
                            if($current_field->systemType eq "MMG_VAR_DATA")
                            {
                                my $current_field_size = $current_field->size;
#--------------------------------------------------------------------------------------------------------
#getVAR_DATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."void $function_print_name(uMDInt * data, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(uMDInt * data, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        for(int i = 0; i < RS_DATA_SIZE; i++)
        {
            *data = ((*(mData->getAddr(ID) + $current_field_offset + i) & $current_field_mask) >> $current_field_shifter);
            data++;
        }
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#setVAR_DATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."void $function_print_name(uMDInt * data, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(uMDInt * data, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // clear first
        for(int i = 0; i < $current_field_size; i++)
        {
            (*(mData->getAddr(ID) + $current_field_offset + i)) &= $current_field_antimask;
        
            // then set
            (*(mData->getAddr(ID) + $current_field_offset + i)) |= ((*data << $current_field_shifter) &  $current_field_mask);
            data++;
        }
    }
    
EOF

#--------------------------------------------------------------------------------------------------------
#clearVAR_DATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        for(int i = 0; i < $current_field_size; i++)
        {
            (*(mData->getAddr(ID) + $current_field_offset + i)) &= $current_field_antimask;
        }
    }
    
EOF

                            }
                            if($current_field->systemType eq "MMG_S_COUNTER")
                            {
                                my $counter_inc = $current_field->c_inc;
                                my $counter_sign = $current_field->c_sign;

#--------------------------------------------------------------------------------------------------------
#getSIGNED_COUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."sMDInt $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    sMDInt $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // getting the counter value
        sMDInt ret_value = ((*(mData->getAddr(ID) + $current_field_offset)) & $current_field_mask) >> $current_field_shifter;
        // work out the sign
        if((*(mData->getAddr(ID) + $current_field_offset)) & $counter_sign)
            return (ret_value * -1);
        return ret_value;
    }

EOF

#--------------------------------------------------------------------------------------------------------
#setSIGNED_COUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."void $function_print_name(sMDInt $input_var, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(sMDInt $input_var, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // is the value too large?
        PARANOID_ASSERT_L1(fAbs($input_var) <= ($current_field_mask>>$current_field_shifter));
        // set the counter value
        *(mData->getAddr(ID) + $current_field_offset) =
            ( (*(mData->getAddr(ID) + $current_field_offset) & $current_field_antimask)    // the bits we're not changing
            | ((fAbs($input_var) << $current_field_shifter) & $current_field_mask) );       // the new counter value
        // fix the sign
        if($input_var < 0)
            *(mData->getAddr(ID) + $current_field_offset) |= $counter_sign;
    }

EOF

#--------------------------------------------------------------------------------------------------------
#resetSIGNED_COUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";
print $OUT_CPP <<"EOF";
    /** INLINE FUNCTION (In the header file)
    $longTemplateString   inline void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    **/

EOF

#--------------------------------------------------------------------------------------------------------
#incrementSIGNED_COUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // if all the bits are set its OK
        // but if all the bits but the sign bit are set then its not OK
        PARANOID_ASSERT_L1( ( ( (*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) == $current_field_mask )
                            & (!(*(mData->getAddr(ID) + $current_field_offset) & $counter_sign) ) ) );

        // for -ve numbers
        if(*(mData->getAddr(ID) + $current_field_offset) & $counter_sign)
        {
            // If the counter is at -1
            if( ($counter_inc | $counter_sign) == (*(mData->getAddr(ID) + $current_field_offset) & ($current_field_mask | $counter_sign)) )
            {
                *(mData->getAddr(ID) + $current_field_offset) &= $current_field_antimask;
            }
            else
                *(mData->getAddr(ID) + $current_field_offset) -= $counter_inc;
        }
        else
            *(mData->getAddr(ID) + $current_field_offset) += $counter_inc;
    }

EOF

#--------------------------------------------------------------------------------------------------------
#decrementSIGNED_COUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        // if all the bits are set its  not OK
        // but if all the bits but the sign bit are set then its OK
        PARANOID_ASSERT_L1( (*(mData->getAddr(ID) + $current_field_offset) & ($counter_sign | $current_field_mask) ) == ($counter_sign | $current_field_mask) )
        // for -ve numbers
        if(*(mData->getAddr(ID) + $current_field_offset) & $counter_sign)
            *(mData->getAddr(ID) + $current_field_offset) += $counter_inc;
        else
        {
            // If the counter is at 0
            if( 0 == (*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) )
                *(mData->getAddr(ID) + $current_field_offset) |= ($counter_inc | $counter_sign);
            else
                *(mData->getAddr(ID) + $current_field_offset) -= $counter_inc;
        }
    }

EOF

                            }
                            if($current_field->systemType eq "MMG_COUNTER")
                            {
                                my $counter_inc = $current_field->c_inc;

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."uMDInt $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    uMDInt $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // getting the counter value
        return ((*(mData->getAddr(ID) + $current_field_offset)) & $current_field_mask) >> $current_field_shifter;
    }

EOF

#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."void $function_print_name(uMDInt $input_var, $this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name(uMDInt $input_var, $this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        // set the counter value
        PARANOID_ASSERT_L1($input_var <= ($current_field_mask>>$current_field_shifter));
        *(mData->getAddr(ID) + $current_field_offset) =
           ( (*(mData->getAddr(ID) + $current_field_offset) & $current_field_antimask)    // the bits we're not changing
            | (($input_var << $current_field_shifter) & $current_field_mask) );       // the new counter value
    }

EOF

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";
print $OUT_CPP <<"EOF";
    /** INLINE FUNCTION (In the header file)
    $longTemplateString   inline void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    **/

EOF

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        PARANOID_ASSERT_L1((*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) != $current_field_mask);
        // increment the counter value
        *(mData->getAddr(ID) + $current_field_offset) += $counter_inc;
    }

EOF

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."void $function_print_name($this_ID_type ID);\n";
print $OUT_CPP <<"EOF";
$longTemplateString
    void $sc_name$shortTemplateString\:\:$function_print_name($this_ID_type ID)
    {
        //-----
        // $function_comment
        //
        PARANOID_ASSERT_L1(mData->getAddr(ID) != MM_ADDR_NULL);
        PARANOID_ASSERT_L1((*(mData->getAddr(ID) + $current_field_offset) & $current_field_mask) != 0);
        // decrement the counter value
        *(mData->getAddr(ID) + $current_field_offset) -= $counter_inc;
    }

EOF

                            }
                        }
print $OUT_H <<"EOF";

    private:

    $mem_manager_name$shortTemplateString * mData;
};

EOF
                        
                    }
                }
            }

            if($current_mc->isTemplated == 1)
            {
                print $OUT_CPP "#endif\n";
            }
            else
            {
                print $OUT_CPP "\n";
            }
            close $OUT_CPP;

            if($current_mc->isTemplated == 1)
            {
                print $OUT_H "#include \"$cpp_file\"\n";
            }
            print $OUT_H "#endif // $if_def_string\n\n";

            close $OUT_H;
        }
    }
}

#
# Automatically code the wrapper template
#
sub auto_code_Wrapper
{
    my $OUT_CPP;                                                    # .cpp file
    my $OUT_H;                                                      # .h file
    my $tab = "    ";                                               # tab character
    
    while (( my $mc_name, my $mc_ref) = each(%masterClasses))
    {
        # get the master class and check it needs autocoding
        my $current_mc = @{$mc_ref}[0];
        if(1 == $current_mc->autoCode)
        {
            # for templated objects we need to work out what the template string will look like
            my $longTemplateString = "";
            my $shortTemplateString = "";
            if($current_mc->isTemplated == 1)
            {
                $longTemplateString = "template <";
                $shortTemplateString = "<";
                my $first_run = 1;
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    if($first_run == 1) { $first_run = 0; }
                    elsif($first_run == 0) { $longTemplateString .= ", "; $shortTemplateString .= ", "; }
                    my $current_tp = @{$tp_ref}[0];
                    $longTemplateString .= $current_tp->type." ".$current_tp->name;
                    $shortTemplateString .= $current_tp->name;
                }
                $longTemplateString .=">";
                $shortTemplateString .=">";
            }

            my $mem_manager_name = $current_mc->name.$layer_1_suffix;
            my $wrapper_name = $current_mc->name.$layer_2_suffix;
            my $wrapper_name_keep = $wrapper_name;                         # keep a copy of the wrapper_name
            my $h_file = "$wrapper_name.h";
            my $if_def_string = $mc_name."_Wrapper_h";
            
            # user modifyable file names
            my $user_editable_file_name;                    # users can modify this file to add methods to the wrapper
            my $user_editable_h_file_name;                  # autogenerated by make from .cpp.extended - general header
            my $user_editable_private_vars_file_name;       # extended private variables
            my $user_editable_includes_file_name;           # extra includes

            # only for templated wrappers
            my $user_editable_virt_h_file_name = "";        # autogenerated by make from .cpp.extended - virtual header
            my $template_extensions_name = "";              # users can modify this file to add new values for the templates

            if($current_mc->isTemplated == 1) {
                $user_editable_file_name = "$wrapper_name.extended.h";
                $template_extensions_name = "$wrapper_name.tempConf.h";
                $user_editable_virt_h_file_name = "$wrapper_name.extended.virt.hm";
                $user_editable_h_file_name = "$wrapper_name.extended.autoheader.hm";
                $user_editable_private_vars_file_name = "$wrapper_name.extended.priVar.hm";
                $user_editable_includes_file_name = "$wrapper_name.extended.includes.hm";
            } else {
                $user_editable_file_name = "$wrapper_name.cpp";
                $user_editable_h_file_name = "$wrapper_name.autoheader.hm";
                $user_editable_private_vars_file_name = "$wrapper_name.priVar.hm";
                $user_editable_includes_file_name = "$wrapper_name.includes.hm";
            }

            # Don't include the h_file if we're templating
            my $h_include = "";
            my $lower_include = "";
            if($current_mc->isTemplated != 1) {
                $h_include = "// system includes\n#include <iostream>\n\n// local includes\n#include \"intdef.h\"\n#include \"IdTypeDefs.h\"\n#include \"$h_file\"\n\nusing namespace std;";
            } else {
                # need to include all the source at the bottom of a templated header file
                $lower_include = "    // for user defined functions (extension functions)\n    #include \"$user_editable_file_name\"\n";
            }
            
            open ($OUT_CPP, ">$user_editable_file_name.template") || die "Can't write to file [$user_editable_file_name.template]: $!.\n";
            push @all_files_written, "$user_editable_file_name.template";

            open ($OUT_H, ">$h_file") || die "Can't write to file [$h_file]: $!.\n";
            push @all_files_written, $h_file;


            

print $OUT_H <<"EOF";
/******************************************************************************
**
** File: $h_file
**
*******************************************************************************
**
** Layer 2 Header file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It serves as a wrapper to the layer 1 code to allow for more easy
** object orientated coding style
**
** DO NOT EDIT THIS FILE.
**
** When you need to add functions then edit only the file: $user_editable_file_name
** and if neccesary $template_extensions_name.
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

#ifndef $if_def_string
    #define $if_def_string

// system includes 
#include <iostream>
#include <vector>

// local includes
#include "intdef.h"
#include "IdTypeDefs.h"
#include "$mem_manager_name.h"

// use defined includes
#include "$user_editable_includes_file_name"

using namespace std;
EOF

            if($current_mc->isTemplated != 1)
            {
                print $OUT_H "class $wrapper_name {\n    public:\n";
            }
            else
            {
                # We need to add the function which will allow us to
                # create this guy in a super sexy way
                # this is a pretty nasty C preProcessor macro
                # there's really no way to do this nicely (although Dom will prove me wrong here! ;)
                # 
                # Here is an example output given the following template params in the .conf file:
                # 
                # T:RL:int:31:Variable length reads
                # T:OFFSET:int:4:Default max offset
                #
                #   #define MAKE_READSTOREMEMWRAPPER(oBJpOINTER,giVeNRL,teSTRL,giVeNOFFSET,teSTOFFSET) { \
                #       if((giVeNRL == teSTRL) && (giVeNOFFSET == teSTOFFSET)){ \
                #           oBJpOINTER = new ReadStoreMemWrapperFixed<teSTRL,teSTOFFSET>();\
                #       }\
                #   }
                #   printed by the immediately following code:
                #   

                print $OUT_H "\n#define MAKE_".uc($wrapper_name)."(oBJpOINTER";
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H ",giVeN".$current_tp->name.",teST".$current_tp->name;
                }
                print $OUT_H ") { \\\n    if(";
                @tp_array = @{$current_mc->templateParams};
                my $first_run = 1;
                foreach my $tp_ref (@tp_array)
                {
                    if($first_run == 1) { $first_run = 0; }
                    else { print $OUT_H " && "; }
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H "(giVeN".$current_tp->name." == teST".$current_tp->name.")";
                }
                print $OUT_H "){ \\\n        oBJpOINTER = new $wrapper_name"."Fixed<";
                @tp_array = @{$current_mc->templateParams};
                 $first_run = 1;
                foreach my $tp_ref (@tp_array)
                {
                    if($first_run == 1) { $first_run = 0; }
                    else { print $OUT_H ","; }
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H "teST".$current_tp->name;
                }
                print $OUT_H ">();\\\n    }\\\n}\n\n";

                # include the user generated file
                print $OUT_H "    #include \"$template_extensions_name\"\n\n";
                
                #
                #   #define makeAppropriateReadStoreMemWrapper(oBJpOINTER,iNSTaNCeRL,iNSTaNCeOFFSET) { \
                #       MAKE_READSTOREMEMWRAPPER(oBJpOINTER,iNSTaNCeRL,31,iNSTaNCeOFFSET,4); \
                #   }
                #   
                #   printed by the immediately following code
                #   

                my $TMP_TMP;
                my $ifdef_template_name = $mc_name."_TEMPLATES";
                open ($TMP_TMP, ">$template_extensions_name.template") || die "Can't write to file [$template_extensions_name.template]: $!.\n";
                push @all_files_written, "$template_extensions_name.template";

print $TMP_TMP <<"EOF";
// template file for adding to the possible values for templates
// copy the contents of this file to $template_extensions_name
// and edit in that file. This file will be overwritten from time to time.
// --------------------------------------------------------------------
// Copyright (C) $copy_right_years $copy_right_holders
//
// This file is part of the Sassy Assembler Project.
//
// Sassy is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Sassy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------
//

#ifndef $ifdef_template_name
    #define $ifdef_template_name
EOF
                print $TMP_TMP "#define makeAppropriate".camelize($wrapper_name)."(oBJpOINTER";
                @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    my $current_tp = @{$tp_ref}[0];
                    print $TMP_TMP ",iNSTaNCe".$current_tp->name;
                }
                print $TMP_TMP ") { \\\n";

                print $TMP_TMP "    MAKE_".uc($wrapper_name)."(oBJpOINTER";
                @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    my $current_tp = @{$tp_ref}[0];
                    print $TMP_TMP ",iNSTaNCe".$current_tp->name.",".$current_tp->default;
                }

                print $TMP_TMP "); \\\n}\n#endif // $ifdef_template_name\n";

                close $TMP_TMP;
                
                print $OUT_H "class $wrapper_name {\n    public:\n";

                # here we need to define the virtual class
print $OUT_H <<"EOF";
  // construct/destruct/intialise/save
    $wrapper_name() {}
    virtual ~$wrapper_name() {}
    virtual bool initialise(idInt originalNumberOfBlocks) { return true; }
    virtual bool save( string fileName ) { return true; }
    virtual bool initialise( string fileName ) { return true; }
EOF
                
                if($current_mc->numSubClasses > 0)
                {
                    my %sc_hash = %{$current_mc->subClasses};
                    while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                    {
                        my $current_sc = @{$sc_ref}[0];
                        my $this_ID_type = $id_hash{$current_sc->classID};
                        my $null_ID = $current_sc->prefix."NULL_ID";
                        
print $OUT_H <<"EOF";

  // NULL IDs
    virtual $this_ID_type getNull$this_ID_type(void) { $this_ID_type foo; return foo; }
  
  // Get/Check $this_ID_type by uMDInts
    virtual inline bool is$this_ID_type(uMDInt address, $this_ID_type ID) { return false; }
    virtual inline $this_ID_type get$this_ID_type(uMDInt address) { $this_ID_type foo; return foo; }
    virtual inline bool isValidAddress($this_ID_type ID) { return true; }

  // New/Free wrappers
    virtual inline $this_ID_type new$this_ID_type(void) { $this_ID_type foo; return foo; }
EOF

                        if(1 == $current_mc->deleteable)
                        {
                            print $OUT_H $tab."inline bool delete$this_ID_type( $this_ID_type ID ) { return true; }\n"
                        }

# Get/Set/Etc.. methods

print $OUT_H "  // Get/Set/etc.. methods\n";

                        my @field_array = @{$current_sc->fields};
                        foreach my $field_ref (@field_array)
                        {
                            my $current_field = @{$field_ref}[0];
                            # put the field name in camel case
                            my $function_base_name = camelize(lc($current_field->name));    # suffix for all functions
                            my $function_print_name;                                        # actual print name
                            my $function_comment =  $current_field->comment;                # comment for the function
                            my $input_var =  lc($current_field->name);                      # name of any input var
                            my $IO_ID_type = $id_hash{$current_field->classID};             # type of any input or output var

                            if($current_field->systemType eq "MMG_FLAG")
                            {
    
#--------------------------------------------------------------------------------------------------------
# isFLAG
                                $function_print_name = "is".$function_base_name;
                                print $OUT_H $tab."virtual inline bool $function_print_name($this_ID_type ID) { return true; }\n";
#--------------------------------------------------------------------------------------------------------
#setFLAG
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(bool $input_var, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#clearFLAG
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

                            }
                            if($current_field->systemType eq "MMG_POINTER")
                            {

#--------------------------------------------------------------------------------------------------------
#getPOINTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."virtual inline $IO_ID_type $function_print_name($this_ID_type ID) { $IO_ID_type foo; return foo;}\n";
    
#--------------------------------------------------------------------------------------------------------
#setPOINTER
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($IO_ID_type $input_var, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#clearPOINTER
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

                            }
                            if($current_field->systemType eq "MMG_DATA")
                            {

#--------------------------------------------------------------------------------------------------------
#getDATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."virtual inline uMDInt $function_print_name($this_ID_type ID) { return 0; }\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(uMDInt data, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

                            }
                            if($current_field->systemType eq "MMG_VAR_DATA")
                            {

#--------------------------------------------------------------------------------------------------------
#getDATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(uMDInt * data, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(uMDInt * data, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

                            }
                            if($current_field->systemType eq "MMG_S_COUNTER")
                            {
                                my $counter_inc = $current_field->c_inc;
                                my $counter_sign = $current_field->c_sign;

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."virtual inline sMDInt $function_print_name($this_ID_type ID) { return 0; }\n";

#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(sMDInt $input_var, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";
                            }
                            if($current_field->systemType eq "MMG_COUNTER")
                            {

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."virtual inline uMDInt $function_print_name($this_ID_type ID) { return 0; }\n";

#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."virtual inline void $function_print_name(uMDInt $input_var, $this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."virtual inline void $function_print_name($this_ID_type ID) {}\n";
                            }
                        }      
                    }
                }
                # for home grown functions we'll need to remake the virtual functions
                print $OUT_H "\n  // for user defined method's virtual functions\n";
                print $OUT_H "  // autogenerated by make\n";
                print $OUT_H "#include \"$user_editable_virt_h_file_name\"\n";
                
                # now begin the class
                print $OUT_H "};\n\n$longTemplateString\nclass $wrapper_name"."Fixed : public $wrapper_name {\n    public:\n";

                # update the wrapper name
                $wrapper_name .= "Fixed";
            }

print $OUT_H <<"EOF";
    // Constructor / Destructor
    
    $wrapper_name() {
        //----
        // Just set all the autocoded guys to NULL
        //
        mData = NULL;
EOF
            $mem_manager_name = $current_mc->name.$layer_1_suffix.$shortTemplateString;
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    print $OUT_H "        m$sc_name = NULL;\n";
                }
            }
print $OUT_H <<"EOF";
    }

    ~$wrapper_name() {
        //----
        // We delete the MemManager here
        // we are the only ones who ever do this!
        //
        extendedDestructor();   // call the extended destructor
        if (mData != NULL) { delete mData; }
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    print $OUT_H "        if (m$sc_name != NULL) { delete m$sc_name; }\n";
                }
            }
print $OUT_H <<"EOF";
    }

    //
    // initialising n' saving n' loading
    //

    bool initialise(idInt originalNumberOfBlocks) {
        //----
        // Call the constructors for the heavies!
        //
        mData = new $mem_manager_name;
        PARANOID_ASSERT_L1(mData != NULL);
        if(!mData->initialise(originalNumberOfBlocks)) { return false; }

EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    my $null_ID = $current_sc->prefix."NULL_ID";

print $OUT_H <<"EOF";
        m$sc_name = new $sc_name$shortTemplateString(mData);
        PARANOID_ASSERT_L2(NULL != m$sc_name);
        
        $null_ID = new$this_ID_type();
        mData->wrapPointer(0, &$null_ID);

EOF
                }
            }
print $OUT_H <<"EOF";
        if(!extendedInitialise(originalNumberOfBlocks)) { return false; }
        return true;
    }
    
    bool initialise( string fileName ) {
        //-----
        // Loading from file
        //
        mData = new $mem_manager_name;
        PARANOID_ASSERT_L1(mData != NULL);
        if(!mData->initialise( fileName + ".MemManager" )) { return false; }
        
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
print $OUT_H <<"EOF";
        m$sc_name = new $sc_name$shortTemplateString(mData);
        PARANOID_ASSERT_L2(NULL != m$sc_name);
        
EOF
                }
            }
print $OUT_H <<"EOF";
        // open the file
        std::ifstream myFile(fileName.c_str(), std::ifstream::binary);

        // load the NULL Ids
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    my $null_ID = $current_sc->prefix."NULL_ID";
                    print $OUT_H "        $null_ID = $this_ID_type(&myFile);\n";
                }
            }
print $OUT_H <<"EOF";

        // close the file
        myFile.close();

        // extended intialisation
        if(!extendedInitialise(fileName + ".extd")) { return false; }
        return true;
    }

    bool save( string fileName ) {
        //-----
        // Save to file
        //
        // open the file
        std::ofstream myFile(fileName.c_str(), std::ofstream::binary);

        // save the NULL Ids
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    my $null_ID = $current_sc->prefix."NULL_ID";
                    print $OUT_H "        $null_ID.save(&myFile);\n";
                }
            }
print $OUT_H <<"EOF";
    
        // close the file
        myFile.close();

        // extended saves
        if(!extendedSave(fileName + ".extd")) { return false; }

        // lower level saves
        return mData->save( fileName + ".MemManager" );
    }
    
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    my $null_ID = $current_sc->prefix."NULL_ID";

print $OUT_H <<"EOF";

/******************************************************************************
** $sc_name functions
******************************************************************************/

  // NULL IDs
    $this_ID_type getNull$this_ID_type(void) { return $null_ID; }
  
  // Get/Check $this_ID_type by uMDInts
    inline bool is$this_ID_type(uMDInt address, $this_ID_type ID) { return mData->is$this_ID_type(address, ID); }
    inline $this_ID_type get$this_ID_type(uMDInt address) { return mData->get$this_ID_type(address); }
    inline bool isValidAddress($this_ID_type ID) { return mData->isValidAddress(ID); }

  // New/Free wrappers
    inline $this_ID_type new$this_ID_type(void) {
        #ifdef MAKE_PARANOID
            $this_ID_type tmpId = m$sc_name\->new$this_ID_type();
            return tmpId;
        #else
            return m$sc_name\->new$this_ID_type();
        #endif
    }
EOF

                    if(1 == $current_mc->deleteable)
                    {
                        print $OUT_H $tab."inline bool delete$this_ID_type( $this_ID_type ID ) { return  m$sc_name\->delete$this_ID_type( ID ); }\n"
                    }

# Get/Set/Etc.. methods

print $OUT_H "\n  // Get/Set/etc.. methods\n";

                    my @field_array = @{$current_sc->fields};
                    foreach my $field_ref (@field_array)
                    {
                        my $current_field = @{$field_ref}[0];
                        # put the field name in camel case
                        my $function_base_name = camelize(lc($current_field->name));    # suffix for all functions
                        my $function_print_name;                                        # actual print name
                        my $function_comment =  $current_field->comment;                # comment for the function
                        my $input_var =  lc($current_field->name);                      # name of any input var
                        my $IO_ID_type = $id_hash{$current_field->classID};             # type of any input or output var

                        print $OUT_H "\n$tab/*\n$tab ** $function_comment\n$tab*/\n";
                            
                        if($current_field->systemType eq "MMG_FLAG")
                        {

#--------------------------------------------------------------------------------------------------------
# isFLAG
                            $function_print_name = "is".$function_base_name;
                            print $OUT_H $tab."inline bool $function_print_name($this_ID_type ID) { return m$sc_name\->$function_print_name(ID); }\n";
#--------------------------------------------------------------------------------------------------------
#setFLAG
                            $function_print_name = "set".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name(bool $input_var, $this_ID_type ID) { m$sc_name\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearFLAG
                            $function_print_name = "clear".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

                        }
                        if($current_field->systemType eq "MMG_POINTER")
                        {

#--------------------------------------------------------------------------------------------------------
#getPOINTER
                            $function_print_name = "get".$function_base_name;
                            print $OUT_H $tab."inline $IO_ID_type $function_print_name($this_ID_type ID) { return m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setPOINTER
                            $function_print_name = "set".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($IO_ID_type $input_var, $this_ID_type ID) { m$sc_name\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearPOINTER
                            $function_print_name = "clear".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

                        }
                        if($current_field->systemType eq "MMG_DATA")
                        {
    
#--------------------------------------------------------------------------------------------------------
#getDATA
                            $function_print_name = "get".$function_base_name;
                            print $OUT_H $tab."inline uMDInt $function_print_name($this_ID_type ID) { return m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                            $function_print_name = "set".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name(uMDInt data, $this_ID_type ID) { m$sc_name\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                            $function_print_name = "clear".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

                        }
                        if($current_field->systemType eq "MMG_VAR_DATA")
                        {
    
#--------------------------------------------------------------------------------------------------------
#getDATA
                            $function_print_name = "get".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name(uMDInt * data, $this_ID_type ID) { return m$sc_name\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                            $function_print_name = "set".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name(uMDInt * data, $this_ID_type ID) { m$sc_name\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                            $function_print_name = "clear".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

                        }
                        if($current_field->systemType eq "MMG_S_COUNTER")
                        {
                                my $counter_inc = $current_field->c_inc;
                                my $counter_sign = $current_field->c_sign;

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                            $function_print_name = "get".$function_base_name;
                            print $OUT_H $tab."inline sMDInt $function_print_name($this_ID_type ID) { return m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                            $function_print_name = "set".$function_base_name;
                            my $setCOUNTER_func_print_name = $function_print_name;
                            print $OUT_H $tab."inline void $function_print_name(sMDInt $input_var, $this_ID_type ID) { m$sc_name\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                            $function_print_name = "reset".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                            $function_print_name = "inc".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                            $function_print_name = "dec".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";
                        }
                        if($current_field->systemType eq "MMG_COUNTER")
                        {

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                            $function_print_name = "get".$function_base_name;
                            print $OUT_H $tab."inline uMDInt $function_print_name($this_ID_type ID) { return m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                            $function_print_name = "set".$function_base_name;
                            my $setCOUNTER_func_print_name = $function_print_name;
                            print $OUT_H $tab."inline void $function_print_name(uMDInt $input_var, $this_ID_type ID) { m$sc_name\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                            $function_print_name = "reset".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                            $function_print_name = "inc".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                            $function_print_name = "dec".$function_base_name;
                            print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { m$sc_name\->$function_print_name(ID); }\n";
                        }
                    }      
                }
            }

print $OUT_H <<"EOF";

    // for user defined functions (autogenerated headers)
    #include "$user_editable_h_file_name"
    
    private:
    
    // for user defined functions (private variables)
    #include "$user_editable_private_vars_file_name"

    $mem_manager_name * mData;
EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    my $current_sc = @{$sc_ref}[0];
                    my $null_ID = $current_sc->prefix."NULL_ID";
                    my $this_ID_type = $id_hash{$current_sc->classID};
                    print $OUT_H "    $sc_name$shortTemplateString * m$sc_name;\n";
                    print $OUT_H "    $this_ID_type $null_ID;\n";
}
            }

print $OUT_H <<"EOF";
};
    $lower_include
#endif // $if_def_string
EOF

            #finally generate the template for the user modifyable code extensions
            my $vo_str1 = "";
            my $vo_str2 = "";
            my $line_break = "//HO ";
            if($current_mc->isTemplated == 1) {
                $vo_str1 = "\n//VO virtual\n$longTemplateString";
                $vo_str2 = "//VO = 0;\n";
                $line_break .= "\n//VO ";
            }            
print $OUT_CPP <<"EOF";
/******************************************************************************
**
** File: $user_editable_file_name.template
**
*******************************************************************************
**
** This file has been automatically generated using maike.pl.
** This file is a template for how you can extend code in the memWrapper
** 
** There is a very specific format for editing this file
** Functions are written in this format and then make can auto generate
** the header files. The functions extendedInitialise and extendedSave are
** generated here by default. Add to these to add variables to be saved when
** save is called
** 
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/
$h_include

/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/
$vo_str1
/*HV*/ void
$wrapper_name$shortTemplateString\:\:
/*HV*/ extendedDestructor(void)
//HO ;
$vo_str2\{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
$line_break
$vo_str1
/*HV*/ bool
$wrapper_name$shortTemplateString\:\:
/*HV*/ extendedInitialise(idInt originalNumberOfBlocks )
//HO ;
$vo_str2\{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    return true;
}
$line_break
$vo_str1
/*HV*/ bool
$wrapper_name$shortTemplateString\:\:
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
$vo_str2\{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    return true;
}
$line_break
$vo_str1
/*HV*/ bool
$wrapper_name$shortTemplateString\:\:
/*HV*/ extendedSave(std::string fileName)
//HO ;
$vo_str2\{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
$line_break

/******************************************************************************
** EXTENSIONS
******************************************************************************/

EOF
        }
    }
}

#
# Automatically code the class template
#
sub auto_code_Class
{
    my $OUT_CPP;                                                    # .cpp file
    my $OUT_H;                                                      # .h file
    my $tab = "    ";                                               # tab character
    
    while (( my $mc_name, my $mc_ref) = each(%masterClasses))
    {
    # get the master class and check it needs autocoding
        my $current_mc = @{$mc_ref}[0];
        if(1 == $current_mc->autoCode)
        {
            my $wrapper_name = $current_mc->name.$layer_2_suffix;
            my $class_name = $current_mc->name.$layer_3_suffix;
            my $cpp_file = "$class_name.cpp";
            my $h_file = "$class_name.h";
            open ($OUT_CPP, ">$cpp_file.template") || die "Can't write to file [$cpp_file.template]: $!.\n";
            open ($OUT_H, ">$h_file") || die "Can't write to file [$h_file]: $!.\n";
            push @all_files_written, "$cpp_file.template";
            push @all_files_written, $h_file;

            my $line_break = "//HO \n";


print $OUT_CPP <<"EOF";
/******************************************************************************
**
** File: $cpp_file.template
**
*******************************************************************************
**
** Layer 3 Implementation file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It serves as an interface to the main algorithm. It interfaces with the
** the wrapper class so all the nitty gritty layer 1 and 2 code is hidden away
** safely.
**
** You will need to edit this file by hand to extend the functionality
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

// system includes
#include <iostream>

// local includes
#include "$wrapper_name.h"
#include "intdef.h"
#include "IdTypeDefs.h"
#include "$h_file"

using namespace std;


/******************************************************************************
** #INCLUDES #DEFINES AND PRIVATE VARS
******************************************************************************/

/******************************************************************************
** INTIALISATION AND SAVING
******************************************************************************/

/*HV*/ void
$class_name\:\:
/*HV*/ extendedDestructor(void)
//HO ;
{
    //-----
    // Add anything you would like to happen when the destructor is called
    //
}
$line_break
/*HV*/ bool
$class_name\:\:
/*HV*/ extendedInitialise(idInt originalNumberOfBlocks )
//HO ;
{
    //-----
    // Add anything you would like to happen on a regular intialisation call
    //
    return true;
}
$line_break
/*HV*/ bool
$class_name\:\:
/*HV*/ extendedInitialise(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when reloading a saved instance
    //
    return true;
}
$line_break
/*HV*/ bool
$class_name\:\:
/*HV*/ extendedSave(std::string fileName)
//HO ;
{
    //-----
    // Add anything you would like to happen when saving (including saving private vars)
    //
    return true;
}
$line_break

/******************************************************************************
** EXTENSIONS
******************************************************************************/

EOF

            my $if_def_string = $mc_name."_Class_h";
print $OUT_H <<"EOF";
/******************************************************************************
**
** File: $h_file
**
*******************************************************************************
**
** Layer 3 Header file for use with projects including MemManager.cpp
**
** This file has been automatically generated using maike.pl.
** It serves as an interface to the main algorithm. It interfaces with the
** the wrapper class so all the nitty gritty layer 1 and 2 code is hidden away
** safely.
**
** DO NOT EDIT THIS FILE. edit ___XXX___Class.cpp only
**
*******************************************************************************
**
**                        A
**                       A B
**                      A B R
**                     A B R A
**                    A B R A C
**                   A B R A C A
**                  A B R A C A D
**                 A B R A C A D A
**                A B R A C A D A B
**               A B R A C A D A B R
**              A B R A C A D A B R A
**
*******************************************************************************
**
** Copyright (C) $copy_right_years $copy_right_holders
**
** This file is part of the Sassy Assembler Project.
**
** Sassy is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Sassy is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Sassy.  If not, see <http://www.gnu.org/licenses/>.
**
******************************************************************************/

#ifndef $if_def_string
    #define $if_def_string

// system includes 
#include <iostream>
#include <vector>

// local includes
#include "$wrapper_name.h"
#include "intdef.h"
#include "IdTypeDefs.h"

// use defined includes
#include "$class_name.includes.hm"

using namespace std;

class $class_name {
    public:
    // Constructor / Destructor
    
EOF
            print $OUT_H "    $class_name(";
            if($current_mc->isTemplated == 1)
            {
                my $first_run = 1;
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    if($first_run == 1) { $first_run = 0; }
                    else { print $OUT_H ","; }
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H $current_tp->type." ".$current_tp->name;
                }
            }
            print $OUT_H ") {\n";
print $OUT_H <<"EOF";

        //----
        // Just set all the autocoded guys to NULL
        //
        mData = NULL;
EOF
            my $new_data_statement;
            if($current_mc->isTemplated == 1)
            {
                $new_data_statement = "makeAppropriate".camelize($wrapper_name)."(mData";
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H "        m".$current_tp->name." = ".$current_tp->name.";\n";
                    $new_data_statement .= ",m".$current_tp->name;
                }
                $new_data_statement .= ");";
            }
            else
            {
                $new_data_statement = "mData = new $wrapper_name;";
            }
            
print $OUT_H <<"EOF";
    }

    ~$class_name() {
        //----
        // We delete the MemManager here
        // we are the only ones who ever do this!
        //
        extendedDestructor();   // call the extended destructor
        if (mData != NULL) { delete mData; }

    }

    bool initialise(idInt originalNumberOfBlocks) {
        //----
        // Call the constructor for the wrapper
        //
        $new_data_statement
        PARANOID_ASSERT_L3(mData != NULL);
        if(!mData->initialise(originalNumberOfBlocks)) { return false; }
        if(!extendedInitialise(originalNumberOfBlocks)) { return false; }
        return true;
    }

    //
    // saving and loading
    //
    bool save( string fileName ) { if(!extendedSave(fileName + ".extd")) {return false;} return mData->save( fileName + ".MemWrapper" ); }
    bool initialise( string fileName ) {
        $new_data_statement
        PARANOID_ASSERT_L3(mData != NULL);
        if(!mData->initialise( fileName + ".MemWrapper")) { return false; }
        if(!extendedInitialise(fileName + ".extd")) { return false; }
        return true;
    }

EOF
            if($current_mc->numSubClasses > 0)
            {
                my %sc_hash = %{$current_mc->subClasses};
                while(( my $sc_name, my $sc_ref ) = each(%sc_hash) )
                {
                    # we only give pass through for the matching subclass
                    if($sc_name eq $mc_name )
                    {
                        my $current_sc = @{$sc_ref}[0];
                        my $this_ID_type = $id_hash{$current_sc->classID};

print $OUT_H <<"EOF";

/******************************************************************************
** $sc_name functions
******************************************************************************/

    // NULL IDs
    $this_ID_type getNull$this_ID_type(void) { return mData->getNull$this_ID_type(); }

    // Get/Check $this_ID_type by uMDInts
    inline bool is$this_ID_type(uMDInt address, $this_ID_type ID) { return mData->is$this_ID_type(address, ID); }
    inline $this_ID_type get$this_ID_type(uMDInt address) { return mData->get$this_ID_type(address); }
    inline bool isValidAddress($this_ID_type ID) { return mData->isValidAddress(ID); }

    //
    // You need to roll your own New/Free wrappers. They should look like something these
    // and remember not to edit this file!
    // inline $this_ID_type new$this_ID_type(void) { return mData\->new$this_ID_type(); }
    // inline bool delete$this_ID_type($this_ID_type ID) { return mData\->delete$this_ID_type( ID ); }
    //
EOF

# Get/Set/Etc.. methods

print $OUT_H "\n  // Get/Set/etc.. methods\n";

                        my @field_array = @{$current_sc->fields};
                        foreach my $field_ref (@field_array)
                        {
                            my $current_field = @{$field_ref}[0];
                            # put the field name in camel case
                            my $function_base_name = camelize(lc($current_field->name));    # suffix for all functions
                            my $function_print_name;                                        # actual print name
                            my $function_comment =  $current_field->comment;                # comment for the function
                            my $input_var =  lc($current_field->name);                      # name of any input var
                            my $IO_ID_type = $id_hash{$current_field->classID};             # type of any input or output var

                            print $OUT_H "\n$tab/*\n$tab ** $function_comment\n$tab*/\n";
                            
                            if($current_field->systemType eq "MMG_FLAG")
                            {

#--------------------------------------------------------------------------------------------------------
# isFLAG
                                $function_print_name = "is".$function_base_name;
                                print $OUT_H $tab."inline bool $function_print_name($this_ID_type ID) { return mData\->$function_print_name(ID); }\n";
#--------------------------------------------------------------------------------------------------------
#setFLAG
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name(bool $input_var, $this_ID_type ID) { mData\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearFLAG
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

                            }
                            if($current_field->systemType eq "MMG_POINTER")
                            {

#--------------------------------------------------------------------------------------------------------
#getPOINTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."inline $IO_ID_type $function_print_name($this_ID_type ID) { return mData\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setPOINTER
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($IO_ID_type $input_var, $this_ID_type ID) { mData\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearPOINTER
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

                            }
                            if($current_field->systemType eq "MMG_DATA")
                            {

#--------------------------------------------------------------------------------------------------------
#getDATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."inline uMDInt $function_print_name($this_ID_type ID) { return mData\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name(uMDInt data, $this_ID_type ID) { mData\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

                            }
                            if($current_field->systemType eq "MMG_VAR_DATA")
                            {

#--------------------------------------------------------------------------------------------------------
#getDATA
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name(uMDInt * data, $this_ID_type ID) { return mData\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#setDATA
                                $function_print_name = "set".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name(uMDInt * data, $this_ID_type ID) { mData\->$function_print_name(data, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#clearDATA
                                $function_print_name = "clear".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

                            }
                            if($current_field->systemType eq "MMG_S_COUNTER")
                            {
                                my $counter_inc = $current_field->c_inc;
                                my $counter_sign = $current_field->c_sign;

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."inline sMDInt $function_print_name($this_ID_type ID) { return mData\->$function_print_name(ID); }\n";
    
#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."inline void $function_print_name(sMDInt $input_var, $this_ID_type ID) { mData\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";
                            }

                            if($current_field->systemType eq "MMG_COUNTER")
                            {

#--------------------------------------------------------------------------------------------------------
#getCOUNTER
                                $function_print_name = "get".$function_base_name;
                                print $OUT_H $tab."inline uMDInt $function_print_name($this_ID_type ID) { return mData\->$function_print_name(ID); }\n";
    
#--------------------------------------------------------------------------------------------------------
#setCOUNTER
                                $function_print_name = "set".$function_base_name;
                                my $setCOUNTER_func_print_name = $function_print_name;
                                print $OUT_H $tab."inline void $function_print_name(uMDInt $input_var, $this_ID_type ID) { mData\->$function_print_name($input_var, ID); }\n";

#--------------------------------------------------------------------------------------------------------
#resetCOUNTER
                                $function_print_name = "reset".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) {$setCOUNTER_func_print_name(0, ID);}\n";

#--------------------------------------------------------------------------------------------------------
#incrementCOUNTER
                                $function_print_name = "inc".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";

#--------------------------------------------------------------------------------------------------------
#decrementCOUNTER
                                $function_print_name = "dec".$function_base_name;
                                print $OUT_H $tab."inline void $function_print_name($this_ID_type ID) { mData\->$function_print_name(ID); }\n";
                            }
                        }
                    }
                }

                print $OUT_H "\n    // user made functions to include\n    #include \"$class_name.autoheader.hm\"\n";
            }
print $OUT_H <<"EOF";

    private:

    $wrapper_name * mData;

    // user made private vars
    #include "$class_name.priVar.hm"

EOF
            if($current_mc->isTemplated == 1)
            {
                my @tp_array = @{$current_mc->templateParams};
                foreach my $tp_ref (@tp_array)
                {
                    my $current_tp = @{$tp_ref}[0];
                    print $OUT_H "    ".$current_tp->type." m".$current_tp->name.";\n";
                }
            }

print $OUT_H <<"EOF";
};
#endif // $if_def_string
EOF

        }
    }
}

__DATA__

=head1 NAME
   
   maike.pl

=head1 COPYRIGHT

   Copyright (c) $copy_right_years Michael Imelfort, Dominic Eales

=head1 DESCRIPTION

   maike.pl

   Used for autocoding memory safe objects. Developed for use with the SASSY algorithm.

   This script will output all the .cpp and .h files needed to build the objects described
   in the ___XXX___Defs.conf files. You shoud call this script from the main src directory

   All .cpp and .h files produced rely on the existence of some core source files. Hopefully
   This code will become more portable in the future, for now ask Mike or Dom if you need
   to port it anywhere
      
   Note that running maike.pl with no confO files will achieve nothing!
   
=head1 SYNOPSIS

   maike.pl --temps directory [--confG globalConfFile] [--silent] --confO file[,file,...]


      --help                        Displays basic usage information
      --man                         Displays more detailed information
      --temps                       The directory where templates are located
      --confG string                The name of the Global configuration file
                                    for building the data struct definitions
                                    EX: GlobalDefs.conf
      --confO string[,string,...]   A list of configuration files for individual
                                    objects we need to build
      --silent                      don't print anythig to screen

      If running from the src folder with templates located in the folder ../autocode/templates/
      and the binary in ../autocode/bin/ folder and we wish to build objects Bob and Fred
      
      EX:  ../autocode/bin/maike.pl --temps ../autocode/templates/ --confG GlobalDefs.conf --confO BobDefs.conf,FredDefs.conf


=cut
