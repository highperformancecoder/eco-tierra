#!etierra
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

file mkdir foodwebs
cd foodwebs

for {set t 0} {$t<2655} {incr t $epoch} { 
    set eco ""
    foreach name [lrange $orgnms 1 end] {
	if {$create($name)<$t+$epoch && $ext($name)>=$t} {
	    lappend eco $name
	}
    }
    
    set graph "digraph {"
    foreach namei [lrange $eco 1 end] {
	append graph "o$namei;"
	foreach namej [lrange $eco 1 end] {
	    if [interacts $namei $namej] {
		append graph "o$namei->o$namej;"
	    }
	}
    }
    append graph "}"
    set out [open "g$t" w]
    puts $out $graph
    close $out
}

exit
GUI
.run configure -text "step"
.stop configure -text "reset t" -command {set t 0}

proc simulate {} {
    uplevel #0 {
    .statusbar configure -text "t:$t"
    set eco ""
    foreach name [lrange $orgnms 1 end] {
	if {$create($name)<$t+$epoch && $ext($name)>=$t} {
	    lappend eco $name
	}
    }
    puts stdout $eco
    set graph "digraph {"
    foreach namei [lrange $eco 1 end] {
	foreach namej [lrange $eco 1 end] {
	    if [interacts $namei $namej] {
		append graph "o$namei->o$namej;"
	    }
	}
    }
    append graph "}"
    netgraph foodweb $graph
    set t [expr $t+$epoch]
}
}


