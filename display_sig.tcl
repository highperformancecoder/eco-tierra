#!etierra
use_namespace etierra

set createf [open unique-orgs r]
set orgnms "self000"
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	append orgnms " [lindex $buf 0]"
    }
}

orgnms $orgnms

results.Init results r
results.set_max_elem 5000

#set name $argv(1)
foreach name [lrange [orgnms] 1 end] {
    display_sig $name
}

