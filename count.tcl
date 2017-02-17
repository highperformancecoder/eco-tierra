#!etierra

# read in a list of organisms. For each organism, find out who the parent was,
# then compute relative proportion of neutral transitions out of all possible
# and then add in whether this transition was neutral or not

set createf [open unique-orgs r]
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	lappend orgnms [lindex $buf 0]
    }
}

etierra.orgnms $orgnms
close $createf
etierra.torgs.Init tierra-orgs r
etierra.neutdb.Init neutdb r
etierra.create_inv_neutdb

set orgnms ""
set createf [open createdb r]
while {![eof $createf]} {
    set buf [gets $createf]
    if {[llength $buf]>=2} {
	lappend orgnms [lindex $buf 0]
    }
}
close $createf

etierra.neutdb.Init neutdb r
etierra.create_inv_neutdb

set sum 0
set sumsingle 0
set n 0
set single 0
foreach name $orgnms {
    set f [open src/$name r]
    for {set buf [gets $f]} {[lindex $buf 0]!="genotype:"} {set buf [gets $f]} {}
    set parent [lindex $buf end]
    if [file exists src/$parent] {
	if {[info exists neut_class($name)] && \
		[info exists neut_class($parent)] && \
		"$neut_class($name)"=="$neut_class($parent)"} {
	    incr sum
	    if {[etierra.hop_count $parent $name]==1} {incr sumsingle}
	}
	incr n
	if {[etierra.hop_count $parent $name]==1} {incr single}
#	puts -nonewline stdout "avneut: [expr double($sum)/$n] +/- \
#		[expr sqrt(double($sum)/$n-double($sum*$sum)/($n*$n))] "
	if {$single>0} {
#	puts stdout "av1hop: [expr double($sumsingle)/$n] +/- \
#		[expr sqrt(double($sumsingle)/$single- \
#		double($sumsingle*$sumsingle)/($single*$single))]"
	    puts stdout "$sumsingle $single $n"
	}
    }
    close $f
}
