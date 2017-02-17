#!etierra

file delete torg_headers.dir torg_headers.pag
etierra.torg_headers.Init torg_headers w
set createdb [open createdb w]
set torgs [open tierra-orgs.txt w]

foreach name [exec ls src] {
# etierra code can only handle 7 character organism names, ie lengths up to 
# 9999. Simply ignore any names bigger than this.
    if {[string length $name]>7} {
	puts stderr "Warning, organism $name ignored"
	continue
    }
    set f [open src/$name r]

    for {set buf [gets $f]} {[lindex $buf 0]!="genotype:"} {set buf [gets $f]} {}
    etierra.torg_headers.genotype [lindex $buf 1]
    etierra.torg_headers.parent [lindex $buf 6]

    for {set buf [gets $f]} {[lindex $buf 0]!="Origin:"} {set buf [gets $f]} {}
    etierra.torg_headers.origin [lindex [split [lindex $buf 2] ,] 0]

    for {set buf [gets $f]} {[lindex $buf 0]!="MaxPropPop:"} {set buf [gets $f]} {}
    etierra.torg_headers.MaxPropPop [lindex $buf 1]
    etierra.torg_headers.MaxPropInst [lindex $buf 3]
    etierra.torg_headers.MaxPop [lindex $buf 5]

    if {[etierra.torg_headers.MaxPop] > 1} {
	puts $createdb "$name [etierra.torg_headers.origin]"
    }
    close $f
    etierra.torg_headers.store_header
    puts $torgs $name
}
close $createdb
etierra.torg_headers.close
close $torgs

file delete tierra-orgs tierra-orgs.dir tierra-orgs.pag
etierra.torgs.Init tierra-orgs w
etierra.addall src
etierra.torgs.close

