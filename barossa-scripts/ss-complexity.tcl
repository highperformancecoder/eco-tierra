#!/bin/sh
#PBS -A rks -q single -l walltime=6:0:0
#\
if [ ! -z "$PBS_O_WORKDIR" ]; then cd $PBS_O_WORKDIR; fi
#\
exec ./etierra ss-complexity.tcl
#\
exit

use_namespace etierra

set createf [open unique-orgs r]
set orgnms "self000"
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
#	append orgnms " [lindex $buf 0]"
	lappend orgnms [lindex $buf 0]
	set createdb([lindex $buf 0]) [lindex $buf 1]
	set extinctdb([lindex $buf 0]) [lindex $buf 2]
    }
}

orgnms $orgnms
close $createf
torgs.Init tierra-orgs r

set infodat [open "info.dat" w]

#set name $argv(1)
foreach name [lrange [orgnms] 1 end] {
    init_org $name
    scan $name %d length
    explore_nbhd [constant $length 0] $length

    set ss 0
    foreach site [neut.sites] {set ss [expr $ss+log($site+1)/log(32.0)]}

    puts $infodat "$name $length $createdb($name) $extinctdb($name) [expr $length-$ss]"
    flush $infodat
}

close $infodat
