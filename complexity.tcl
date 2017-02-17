#!etierra
##PBS -A rks -l walltime=6:0:0 -q single
#\
    cd $PBS_O_WORKDIR
#\
    ./etierra complexity.tcl
#\
    exit
# TCL code starts here
use_namespace etierra

set createf [open unique-orgs r]
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=3} {
	set name [lindex $buf 0]
	set create($name) [lindex $buf 1]
	set ext($name) [lindex $buf 2]
	lappend orgnms $name
    }
}

etierra.results.Init results r
etierra.results.set_max_elem 1000

set t 0
set epoch 1

set cdat [open complexity.dat w]

for {set t 0} {$t<3000} {incr t $epoch} { 
    set eco ""
    foreach name [lrange $orgnms 1 end] {
	if {$create($name)<$t+$epoch && $ext($name)>=$t} {
	    lappend eco $name
	}
    }

    orgnms $eco
    puts $cdat "$t [complexity]"
}

close $cdat


