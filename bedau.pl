#!/usr/bin/perl
# phenotype map is generated from a hacked rem-neutrals.tcl script
open PHENOMAP,"<phenomap.dat";
while ($_=<PHENOMAP>)
{
    ($oname,$pheno)=split;
    $phenomap{$oname}=$pheno;
}
close PHENOMAP;

$diversity=0;
$threshold=10; #threshold for deciding if organism is adaptively significant
$t=0;

while (<>)
{
    split;
    $delta_t=hex($_[0]);
    if ($delta_t > 0)
    {
	# output Bedau-Packard stats
	$A_cum=0;
	$A_new=0;
	foreach $org (keys(%pop))
	{
	    $activity{$org}+=$delta_t*$pop{$org};
	    $A_cum+=$activity{$org};
	    if ($pop{$org} > $threshold && !exists $adaptive{$org})
	    {
		$adaptive{$org}=1;
		$A_new++;
	    }
	}
	$Abar_cum=($diversity==0)? 0: $A_cum/$diversity;
	$Abar_new=($diversity==0)? 0: $A_new/$diversity;
	print "$t $diversity $Abar_cum $Abar_new\n";
    }
    $t+=$delta_t;
    shift @_;
    if ($#_>=0 && length($_[0])==1 && $_[0]=~/[a-z]/) 
    {
	$bd=$_[0]; 
	shift @_;
    }
    if ($#_>=0 && $_[0]=~/\d/) 
    {
	# not label, must be length
	$l=$_[0];
	shift @_;
    }
    if ($#_>=0) 
    {
	$tag=$_[0];
    }
#    print "$t | $bd | $l | $tag\n";

    $name=sprintf "%04d%s",$l,$tag;
    if (exists($phenomap{$name}))
    {
	$name=$phenomap{$name};
    }
    if ($bd eq 'b') 
    {
	if (!exists($pop{$name}))
	{
#	    print "$name $t created\n";
	    $pop{$name}=0;
	    $activity{$name}=0;
	    $diversity++;
	}
	$pop{$name}++;
    }
    if ($bd eq 'd') 
    {
	$pop{$name}--;
	if ($pop{$name}==0) 
	{
	    $extinction{$name}=$t;
	    $diversity--;
	    delete $activity{$name};
	    delete $pop{$name};
	}
    }
}

