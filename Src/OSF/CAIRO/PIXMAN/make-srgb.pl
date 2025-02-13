#!/usr/bin/perl -w

# @sobolev use strict;

sub linear_to_srgb
{
    my ($c) = @_;

    if ($c < 0.0031308)
    {
	return $c * 12.92;
    }
    else
    {
	return 1.055 * $c ** (1.0/2.4) - 0.055;
    }
}

sub srgb_to_linear
{
    my ($c) = @_;

    if ($c < 0.04045)
    {
	return $c / 12.92;
    }
    else
    {
	return (($c + 0.055) / 1.055) ** 2.4
    }
}

my @linear_to_srgb;
for my $linear (0 .. 4095)
{
    my $srgb = int(linear_to_srgb($linear / 4095.0) * 255.0 + 0.5);
    push @linear_to_srgb, $srgb;
}

my @srgb_to_linear;
for my $srgb (0 .. 255)
{
    my $linear = int(srgb_to_linear($srgb / 255.0) * 65535.0 + 0.5);
    push @srgb_to_linear, $linear;
}

# Ensure that we have a lossless sRGB and back conversion loop.
# some of the darkest shades need a little bias -- maximum is just
# 5 increments out of 16. This gives us useful property with
# least amount of error in the sRGB-to-linear table, and keeps the actual
# table lookup in the other direction as simple as possible.
for my $srgb (0 .. $#srgb_to_linear)
{
    my $add = 0;
    while (1)
    {
	my $linear = $srgb_to_linear[$srgb];
	my $srgb_lossy = $linear_to_srgb[$linear >> 4];
	last if $srgb == $srgb_lossy;

	# Add slight bias to this component until it rounds correctly
	$srgb_to_linear[$srgb] ++;
	$add ++;
    }
    die "Too many adds at $srgb" if $add > 5;
}

print <<"PROLOG";
/* WARNING: This file is generated by $0.
 * Please edit that file instead of this one.
 */

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pixman-private.h"

PROLOG

print "const uint8_t linear_to_srgb[" . @linear_to_srgb . "] =\n";
print "{\n";
for my $linear (0 .. $#linear_to_srgb)
{
    if (($linear % 10) == 0)
    {
	print "\t";
    }
    print sprintf("%d, ", $linear_to_srgb[$linear]);
    if (($linear % 10) == 9)
    {
	print "\n";
    }
}
print "\n};\n";
print "\n";

print "const uint16_t srgb_to_linear[" . @srgb_to_linear . "] =\n";
print "{\n";
for my $srgb (0 .. $#srgb_to_linear)
{
    if (($srgb % 10) == 0)
    {
	print "\t";
    }
    print sprintf("%d, ", $srgb_to_linear[$srgb]);
    if (($srgb % 10) == 9)
    {
	print "\n";
    }
}
print "\n};\n";

