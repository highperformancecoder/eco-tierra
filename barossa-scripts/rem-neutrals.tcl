##!etierra
#PBS -A rks -q single -l walltime=0:30:0
#\
    cd $PBS_O_WORKDIR
#\
    cp createdb etierra extinctiondb rem-neutrals.tcl torg_headers /scratch/$PBS_JOBID
#\
    if [ -f rem-neutrals.ckpt ]; then \
    scp barossafs:$PBS_O_WORKDIR/rem-neutrals.ckpt /scratch/$PBS_JOBID; fi
#\
    cd /scratch/$PBS_JOBID
#\
scp barossafs:$PBS_O_WORKDIR/results .
#\
    ./etierra rem-neutrals.tcl
# post processing
#\
scp neutdb unique-orgs barossafs:$PBS_O_WORKDIR
#\
    if [ -f rem-neutrals.ckpt ]; then \
    scp rem-neutrals.ckpt barossafs:$PBS_O_WORKDIR; fi
#\
    exit
# TCL input starts here
set clock0 [clock seconds]
# name of checkpoint file
set ckpt "rem-neutrals.ckpt"
use_namespace etierra

proc elapsed {} {
    global clock0
    return [expr [clock seconds]-$clock0]
}

results.Init results r
results.set_max_elem 5000

if [file exists $ckpt] {
    restart $ckpt
} {

    #set MaxPropPop_threshold 0.005
    set MaxPropPop_threshold 0.05
    set createf [open createdb r]
    torg_headers.Init torg_headers r
    while {![eof $createf]} {
	set buf [gets $createf]
	if {[llength $buf]>=2} {
	    set name [lindex $buf 0]
	    torg_headers.load_header $name
	    # "strange" characters in org names cause problems with lappend. 
	    # Use append instead
	    if {[torg_headers.MaxPropPop]>$MaxPropPop_threshold} { 
		append orgnms "[lindex $buf 0] "
		lappend create [lindex $buf 1]
		puts -nonewline "[lindex $buf 0]\r"
	    }
	    append fullorgnms "[lindex $buf 0] "
	}
    }

    puts "orgnms loaded [elapsed] secs"

    set orgnms [string trim $orgnms]
    orgnms $orgnms
    create $create
    set max_time [max create]


    if [file exists extinctiondb] {
	set extinctf [open extinctiondb r]

	while {![eof $extinctf]} {
	    set buf [gets $extinctf]
	    set e [lindex $buf 1]
	    set ext([lindex $buf 0]) $e
	    if {$e>$max_time} {set max_time $e}
	}

	for {set i 0} {$i<[llength $orgnms]} {incr i} {
	    #	puts stdout "[lindex $orgnms $i] $ext([lindex $orgnms $i])"
	    if [info exists ext([lindex $orgnms $i])] {
		lappend extinct $ext([lindex $orgnms $i])
	    } else {
		lappend extinct $max_time
	    }
	}
	
	extinct $extinct
    } else {
	extinct [constant [llength $create] 0]
    }
    

    create_key_exist
    puts stdout "keys created [elapsed] secs"

    file delete neutdb neutdb.dir neutdb.pag fneutdb fneutdb.dir fneutdb.pag

    puts stdout "[llength $orgnms] [llength $create] [llength [extinct]]"

    puts stdout "removing the nonliving"
    etierra.rem_nonliving
    puts stdout "removing neutrals"

    min_i 0
    min_j 0

}

set blksz 1000
neutdb.Init neutdb w

for {} {[min_i]<[norgnms]} {min_i [expr [min_i]+$blksz]} {
    for {} {[min_j]<=[min_i]} {min_j [expr [min_j]+$blksz]} {
	if {[elapsed] > 82000} {
	    checkpoint $ckpt
	    exit
	}
	puts stdout "[min_i] [min_j] [elapsed] secs"
	etierra.rem_neutrals $blksz
    }
}
file delete $ckpt
etierra.cleanup_neutrals
etierra.neutdb.close

set orgnms [etierra.orgnms]
set create [etierra.create]
set extinct [etierra.extinct]

set unique_orgs [open "unique-orgs" w]
for {set i 0} {$i<[llength $orgnms]} {incr i} {
    if {![string equal [lindex $orgnms $i] "self000"]} {
	puts $unique_orgs "[lindex $orgnms $i] [lindex $create $i] [lindex [extinct] $i] "
    }
}

close $unique_orgs

# only used for the missing neutrality job 
# etierra.neutdb.Init fneutdb w
# etierra.build_neutdb $fullorgnms
# etierra.neutdb.close

puts "[elapsed] seconds"
