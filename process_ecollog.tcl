#!etierra
use_namespace etierra
neutdb.Init neutdb r
neutdb.set_max_elem 5000
#interaction_counts.Init interactionsdb w
#interaction_counts.set_max_elem 20
load_ecollog ecol.log
set complexities [open "complexity.dat" w]
for {set t 0} {$t<[maxt]} {incr t 100} {
    foodwebAt $t 200
    foodweb.output pajek foodweb[format "%05d" $t].net
    puts stdout "$t [foodweb.nodes]"
    set c [complexity]
    puts stdout "$t $c"
    puts $complexities "$t $c"
}
