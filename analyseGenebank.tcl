#!etierra
# multiprocessor support
set np 1
set proc 0
if {$argc>2} {
    set np $argv(1)
    set proc $argv(2)
}

use_namespace etierra
genebank.Init genebank r
genebank.max_elem 1000
set orgnms [lsort [genebank.orgList]]

resultDb.Init results$proc w
resultDb.max_elem 1000
resultDbIdx.Init resultIdx$proc w
resultDbIdx.max_elem 1000

soup.tournamentMode 1
soup.setMaxCells 4

####foreach orgA $orgnms {
for {set o $proc} {$o<[llength $orgnms]} {incr o $np} {
    set orgA [lindex $orgnms $o]
    soup.clear
    addEmptyCell
    addEmptyCell
    injectOrg $orgA
    puts "doing $orgA"
    runJoust
    insertResults

    foreach orgB $orgnms {
        soup.clear
# order and spacing in soup reflects etierra.3
# TODO try different injections
        injectOrg $orgB
        addEmptyCell
        injectOrg $orgA
        addEmptyCell
#        puts "doing $orgA $orgB"
# TODO check for interaction
# TODO determine no steps based on cell size
        if [soup.interacts] {
            runJoust
            insertResults
#puts "interacts"
        } 

        if {$orgA == $orgB} {
            break;
        }
    }
}
