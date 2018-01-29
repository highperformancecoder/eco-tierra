#!etierraVCPU16
use_namespace etierra
genebank.Init genebank w
importGenome vectorCPU16bitAncestor1.tie
soup.setMaxCells 1000

proc assert {x args} {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed: $args"
        exit 1
    }
}

# insert the anscestor, and then run the simulator some
injectOrg ancestor
GUI
soup.run 1000
puts [soup.cells.size]

#check that all cells are in a valid state
        
#assert [soup.cells.size]>1
for {set cell 0} {$cell<[soup.cells.size]} {incr cell} {etierra.soup.cells.@elem $cell}
#for {set cell 0} {$cell<[soup.cells.size]} {incr cell} {
#    set c etierra.soup.cells($cell)
#    assert "[$c.cellID] == $cell" 
#    if {[$c.owner] == $cell} {
##        # we have divided from parent
#        assert \[$c.cpu.active\]==1
#        assert \[$c.cpu.divs\]<\[$c.cpu.inst_exec\]
#        assert \[$c.cpu.faults\]<\[$c.cpu.inst_exec\]
#        assert "\[string equal \[$c.organism.name\] ancestor\]"
#        assert "\[string equal \[$c.organism.parent\] god\]"
#        assert \[$c.organism.genome.size\]==80
#        assert "\[$c.organism.genome\]==\"nop1 nop1 nop1 nop1 zero not0 shl shl movDC adrb nop0 nop0 nop0 nop0 subAAC movBA adrf nop0 nop0 nop0 nop1 incA subCAB nop1 nop1 nop0 nop1 mal call nop0 nop0 nop1 nop1 divide jmpo nop0 nop0 nop1 nop0 ifz nop1 nop1 nop0 nop0 pushA pushB pushC nop1 nop0 nop1 nop0 movii decC ifz jmpo nop0 nop1 nop0 nop0 incA incB jmpo nop0 nop1 nop0 nop1 ifz nop1 nop0 nop1 nop1 popC popB popA ret nop1 nop1 nop1 nop0 ifz\""
#        assert \[$c.organism.nDivs\]==\[etierra.soup.cells(0).organism.nDivs\]
#        if {[$c.daughterAllocated]} {
#            # we have malloc'ed, but not divided daughter
#            set dc [etierra.soup.cells.@elem [expr [$c.cpu.daughter]>>9]]
#            assert "\"[$dc.organism.name]\"=={}" dc.organism.name
#            assert [$dc.organism.genome.size]==80 dc.organism.genome.size
#            assert [$dc.cpu.active]==0 dc.cpu.active
#            assert [$dc.cpu.divs]==0 dc.cpu.divs
#        }
#     }  else  {
#         # cell has not been started yet (is inactive).
#         lappend inactive $cell
#         assert \[$c.cpu.divs\]==0
#         assert \[$c.cpu.active\]==0
#         assert \[$c.cpu.inst_exec\]==0
#         assert \[$c.cpu.faults\]==0
#         assert \[$c.organism.genome.size\]==80
#     }
#}

#
## now run the simulator some more, then check that all inactive cells
## become active and divide.
#run 2000
#
#foreach cell $inactive {
#    etierra.soup.cells.@elem $cell
#    set c etierra.soup.cells($cell)
#    assert \[$c.cpu.active\]==1
#    assert \[$c.cpu.divs\]>0
#    assert \[$c.cpu.divs\]<\[$c.cpu.inst_exec\]
#    assert \[$c.cpu.faults\]<\[$c.cpu.inst_exec\]
#    assert "\[string equal \[$c.organism.name\] 0080aaa\]"
#    assert "\[string equal \[$c.organism.parent\] 0666god\]"
#    assert \[$c.organism.genome.size\]==80
#    assert "\[$c.organism.genome\]==\"nop1 nop1 nop1 nop1 zero not0 shl shl movDC adrb nop0 nop0 nop0 nop0 subAAC movBA adrf nop0 nop0 nop0 nop1 incA subCAB nop1 nop1 nop0 nop1 mal call nop0 nop0 nop1 nop1 divide jmpo nop0 nop0 nop1 nop0 ifz nop1 nop1 nop0 nop0 pushA pushB pushC nop1 nop0 nop1 nop0 movii decC ifz jmpo nop0 nop1 nop0 nop0 incA incB jmpo nop0 nop1 nop0 nop1 ifz nop1 nop0 nop1 nop1 popC popB popA ret nop1 nop1 nop1 nop0 ifz\""
#    assert \[$c.organism.nDivs\]==\[etierra.soup.cells(0).organism.nDivs\]
#}
