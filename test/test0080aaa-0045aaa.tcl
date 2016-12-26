#!etierra
use_namespace etierra
genebank.Init genebank w
importGenome 0045aaa.tie
importGenome 0080aaa.tie
soup.setMaxCells 1000

proc assert x {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed"
        exit 1
    }
}

# insert the parasite, and then run the simulator some
injectOrg 0045aaa
run 100

#check that no meaningful divisions have taken place
        
puts [genebank.rambankSize]
assert {[genebank.rambankSize]==1}
puts [genebank.rambankSize]
set r [genebank.rambankElem 0]
assert "\[string equal \"\[$r.name\]\" 0045aaa\]"
assert \[$r.nDivs\]==0
assert \[$r.population\]==1
assert \[$r.maxPop\]==1

#set r [genebank.rambankElem 1]
#assert \[$r.genome.size\]==45
#assert "\[string equal \"\[$r.genome\]\" \"nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0 nop0\"\]"

for {set cell 0} {$cell<[soup.cells.size]} {incr cell} {etierra.soup.cells.@elem $cell}
set c etierra.soup.cells(0)
assert "\[string equal \"\[$c.organism.name\]\" 0045aaa\]"

for {set cell 1} {$cell<[soup.cells.size]} {incr cell} {
    puts stdout "checking cell:$cell"
    set c etierra.soup.cells($cell)
    assert "!\[string equal \"\[$c.organism.name\]\" 0045aaa\]"
}

soup.clear

# checks that both parasite and host divide
injectOrg 0045aaa
injectOrg 0080aaa
run 1000

assert \[genebank.rambankSize\]==2
set r [genebank.rambankElem 0]
assert "\[string equal \"\[$r.name\]\" 0045aaa\]"
assert \[$r.nDivs\]>0
assert \[$r.population\]>1
assert \[$r.maxPop\]==\[$r.population\]

set r [genebank.rambankElem 1]
assert \[$r.genome.size\]==80
assert "\[string equal \"\[$r.name\]\" 0080aaa\]"
assert \[$r.nDivs\]>0
assert \[$r.population\]>1
assert \[$r.maxPop\]==\[$r.population\]

