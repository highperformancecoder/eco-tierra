#!etierra
use_namespace etierra

set createf [open unique-orgs r]
set orgnms "self000"
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	append orgnms " [lindex $buf 0]"
#	lappend orgnms [lindex $buf 0]
	set createdb([lindex $buf 0]) [lindex $buf 1]
	set extinctdb([lindex $buf 0]) [lindex $buf 2]
    }
}

etierra.orgnms $orgnms
broadcastOrgs
close $createf
puts stdout "b4 torgs.Init"
parallel etierra.torgs.Init tierra-orgs r
puts stdout "after torgs.Init"

#set infodat [open "info.dat" w]
set infodat [open "info.dat" a]

set names [open orglist r]

#foreach name [lrange [orgnms] 1 end] 
while {[gets $names name] > 0} {
    puts stdout "doing $name"
    parallel etierra.init_org $name
    scan $name %d length
    puts stdout "explore_nbhd"
    explore_nbhd [constant $length 0] $length

    set ss 0
    foreach site [neut.sites] {set ss [expr $ss+log($site+1)/log(32.0)]}

    puts $infodat "$name $length $createdb($name) $extinctdb($name) [expr $length-$ss]"
    flush $infodat
}

close $infodat
