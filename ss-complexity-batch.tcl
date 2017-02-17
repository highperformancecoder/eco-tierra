#!/bin/sh
#PBS -A rks -q single -l walltime=72:0:0
#\
if [ ! -z "$PBS_O_WORKDIR" ]; then cd $PBS_O_WORKDIR; fi
#\
exec ./etierra ss-complexity-batch.tcl
#\
exit

use_namespace etierra

set createf [open unique-orgs r]
set orgnms "self000"
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	append orgnms " [lindex $buf 0]"
	set createdb([lindex $buf 0]) [lindex $buf 1]
	set extinctdb([lindex $buf 0]) [lindex $buf 2]
    }
}

set orgnms [string trim $orgnms]
orgnms $orgnms
close $createf
torgs.Init tierra-orgs r


set job $env(PBS_JOBNAME)
set infodat [open "info-$job.dat" a]

set createf [open "orgs-$job" r]
set orgnms ""
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	append orgnms " [lindex $buf 0]"
    }
}

set orgnms [string trim $orgnms]

foreach name $orgnms {
    init_org $name
    scan $name %d length
    explore_nbhd [constant $length 0] $length

    set ss 0
    foreach site [neut.sites] {set ss [expr $ss+log($site+1)/log(32.0)]}

    puts $infodat "$name $length $createdb($name) $extinctdb($name) [expr $length-$ss]"
    flush $infodat
}

close $infodat
