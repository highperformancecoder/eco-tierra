#!../netcomplexity
use_namespace etierra
foodweb.input pajek $argv(1)

puts -nonewline stdout "$argv(1) [foodweb.nodes] [foodweb.links] "
set complexity [complexity]
set fmt "%3.1f"
puts -nonewline stdout "[format $fmt $complexity] "
flush stdout

if {[foodweb.nodes]==0} {
    puts stdout 0
} else {
    HistoStats Rewire
    for {set i 1} {$i<100} {incr i} {
        random_rewire
        Rewire.add_data [expr log([complexity])]
    }
    
    #GUI
    #histogram h [Rewire]
    puts -nonewline stdout "[format $fmt [expr exp([Rewire.av])]] [format $fmt [expr $complexity-exp([Rewire.av])]] "
    flush stdout
    if {[Rewire.stddev] != 0} {
        puts stdout "[format $fmt [expr abs(log($complexity)-[Rewire.av])/[Rewire.stddev]]]"
    } else {
        puts stdout "infinity"
    }
}
