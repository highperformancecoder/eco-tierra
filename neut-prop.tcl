#!/bin/sh
##PBS -l walltime=6:0:0,vmem=9GB,ncpus=9
#PBS -l walltime=24:0:0,vmem=1GB,ncpus=1

# read in a list of organisms. For each organism, find out who the parent was,
# then compute relative proportion of neutral transitions out of all possible
# and then add in whether this transition was neutral or not

#\
if [ ! -z "$PBS_O_WORKDIR" ]; then cd $PBS_O_WORKDIR; fi
#\
PATH=/usr/local/pbs/bin:/opt/pbs/bin:$PATH
#\
exec lrun -n 1 etierra neut-prop.tcl
#exec $HOME/usr/bin/mpirun -np 9 etierra neut-prop.tcl

set lastcpu 0

set createf [open unique-orgs r]
set orgnms "self000"
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
#	append orgnms " [lindex $buf 0]"
	lappend orgnms [lindex $buf 0]
    }
}

parallel etierra.orgnms $orgnms
close $createf
parallel etierra.torgs.Init tierra-orgs r
etierra.neutdb.Init fneutdb r
etierra.create_inv_neutdb
#set f [open invneutdb.txt r]
#array set neut_class [gets $f]
#close $f

if [file exists checkpoint] {
    set orgtoggle false
    set c [open checkpoint r]
    gets $c lastname
    gets $c sumneuts
    gets $c sumneutssq 
    gets $c n
    close $c
    set datfile [open neut-prop.dat a]
} else {
    set orgtoggle true
    set datfile [open neut-prop.dat w]
    set sumneuts 0.0
    set sumneutssq 0.0
    set n 0
}

set orgnms ""
set createf [open createdb r]
while {![eof $createf]} {
    gets $createf buf
    if {[llength $buf]>=2} {
	if {$orgtoggle} {
	    append orgnms "[lindex $buf 0] "
	} elseif {"[lindex $buf 0]"=="$lastname"} {
	    set orgtoggle true
	}
    }
}
close $createf

etierra.torg_headers.Init torg_headers r

foreach name $orgnms {
    etierra.torg_headers.load_header $name
    set parent [etierra.torg_headers.parent]    
    etierra.torg_headers.load_header $parent
    if {[etierra.torg_headers.MaxPop]>0 && [etierra.hop_count $name $parent]==1} {
	if {[info exists neut_class($name)] && \
		[info exists neut_class($parent)] && \
		"$neut_class($name)"=="$neut_class($parent)"} { 
	    if {![info exists neutprop($parent)]} {
		parallel etierra.init_org $parent
		scan $parent %d length
		etierra.explore_nbhd [constant $length 0] $length
		set neutprop($parent) [expr [av [etierra.neut.sites]]/32.0]
	    }
	    if {$neutprop($parent)>0} {
		set neutinc [expr 1.0/$neutprop($parent)]
	    } else {set neutinc 0}
	} else {set neutinc 0}
	set sumneuts [expr $sumneuts+$neutinc]
	set sumneutssq [expr $sumneutssq+$neutinc*$neutinc]
	incr n
	set av [expr $sumneuts/$n]
	if {$n>1} {
	    puts $datfile \
		"$name av: $av stddev: [expr sqrt($sumneutssq/($n*$n)-$av*$av/$n)]"
	}
	flush $datfile 
    }
    if {[cputime]-$lastcpu >50000} {
	set lastcpu [cputime]
	set c [open checkpoint w]
	puts $c $name
	puts $c $sumneuts
	puts $c $sumneutssq 
	puts $c $n
	close $c
	close $datfile
	exec qsub neut-prop.tcl &
	exit
    }
}

file delete checkpoint
