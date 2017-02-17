#!etierra
use_namespace etierra
results.Init results r
results.set_max_elem 5000

set orgnmsf [open small-neutclass r]
while {![eof $orgnmsf]} {
    gets $orgnmsf buf
    set name [lindex $buf 0]
    append orgnms "$name "
}

orgnms $orgnms

interactionMatrix interactionMat.dat
