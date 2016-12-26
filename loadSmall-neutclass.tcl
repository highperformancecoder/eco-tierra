#!etierra
use_namespace etierra
genebank.Init genebank w

set orgnmsf [open small-neutclass r]
while {![eof $orgnmsf]} {
    gets $orgnmsf buf
    set name [lindex $buf 0]
    if [string length $name] {
        importGenome src/$name
    }
}

genebank.reverseGenebank.Init genebankR w
genebank.populateReverseGenebank
