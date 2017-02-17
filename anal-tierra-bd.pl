#!/usr/bin/perl
while (<>)
{
    split;
    $t+=hex($_[0]);
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
    if ($bd eq 'b') 
    {
	if (!exists($pop{$name}))
	{
#	    print "$name $t created\n";
	    $pop{$name}=0;
	}
	$pop{$name}++;
    }
    if ($bd eq 'd') 
    {
	$pop{$name}--;
	if ($pop{$name}==0) {$extinction{$name}=$t;}
    }
}

foreach $org (keys %pop)
{
    if (exists($extinction{$org}))
    {
	$text=$extinction{$org};
    }
    else
    {
	$text=$t;
    }
    print "$org ".int($text/1000000) ."\n";
}
