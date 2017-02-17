#!etierra
set MaxPropPop_threshold 0.0001
etierra.torgs.Init tierra-orgs r
etierra.torgs.set_max_elem 10
etierra.torg_headers.Init torg_headers r

if {$argc>=3} {
  set offs $argv(1)
  set incr $argv(2)
  set resultsname results$offs
} else {
  set offs 0
  set incr 1
  set resultsname results
}

set orgnmsf [open createdb r]
while {![eof $orgnmsf]} {
    gets $orgnmsf buf
    set name [lindex $buf 0]
    etierra.torg_headers.load_header $name
    if {[etierra.torg_headers.MaxPropPop]>$MaxPropPop_threshold} { 
	append orgnms "$name "
    }
}

etierra.torg_headers.close

etierra.orgnms $orgnms
puts stdout "[llength $orgnms] tested"

puts stdout [etierra.orgnms]

file delete $resultsname $resultsname.dir $resultsname.pag
etierra.results.Init $resultsname w
etierra.results.set_max_elem 1000

for {set i $offs} {$i<[llength $orgnms]} {incr i $incr} {
    set org [lindex $orgnms $i]
    puts stdout "doing $org"
    etierra.addresults $org
}

etierra.results.close
puts stdout "cputime=[cputime]"
