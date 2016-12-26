#!etierra
use_namespace etierra
genebank.Init genebank w
genebank.max_elem 1000
genebank.reverseGenebank.Init genebankR w
soup.setMaxCells 1000
genebank.populateReverseGenebank
injectOrg 0080aaa
injectOrg 0045aaa
soup.mutRate 1000
soup.flawRate 1000
soup.copyFlawRate 10

if [file exists etierra.ckpt] {
    restart etierra.ckpt
}

proc displayRambank {} {
    for {set i 0} {$i<[genebank.rambank.size]} {incr i} {
        set g [genebank.rambank.@elem $i]
        puts stdout "[$g.name]: genome size=[$g.genome.size], ndivs=[$g.nDivs], maxPop=[$g.maxPop] pop=[$g.population]"
    }
}

proc simulate {} {
    global running
    while {[soup.tstep]<1e10} {
        run 10
#            puts stdout ">>>>>>>>>>>>>>>[soup.tstep]>>>>>>>>>>>>>"
#            displayRambank
#            puts stdout "======================================="
        set orgs [genebank.archiveRambank]
        if {[string length $orgs]>0} {
            puts stdout "extracted: $orgs at [soup.tstep]"
        }
#            displayRambank
#            puts stdout "<<<<<<<<<<<<<<<[soup.tstep]<<<<<<<<<<<<<"
#        .statusbar configure -text "t:[soup.tstep]"
        puts -nonewline stdout  "\r[soup.tstep]"
        flush stdout
        if {[soup.tstep]%100000==0} {
            etierra.checkpoint etierra.ckpt
            genebank.commit
            genebank.reverseGenebank.commit
        }
        update
    }
}

#GUI
simulate
