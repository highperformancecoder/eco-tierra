#!/usr/local/bin/perl
# number of jobs to split work over
$job=$ARGV[0];
$nproc=$ARGV[1];

open infodat,"<info.dat";
while ($_=<infodat>)
{
    ($org)=split;
    $done{$org}=1;
}

open orgdb,"<unique-orgs";
for ($i=0; $i<$nproc; $i++)
{
    open "f$i",">orgs-$job$i";
}

$i=0;
while ($_=<orgdb>)
{
    ($org)=split;    
    if (!exists $done{$org})
    {
	print {"f$i"} "$_";
	$i++;
	if ($i==$nproc) {$i=0};
    }
}
for ($i=0; $i<$nproc; $i++)
{
    close "f$i",">orgs-$job$i";
    system "qsub -N $job$i ss-complexity-batch.tcl"; 
}
