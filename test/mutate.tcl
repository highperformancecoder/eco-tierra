#!etierra
use_namespace etierra
genebank.Init genebank w
importGenome 0080aaa.tie
importGenome 0045aaa.tie
soup.setMaxCells 1000

proc assert x {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed"
        exit 1
    }
}

## insert the anscestor, and then run the simulator some with various flaws
#injectOrg 0080aaa
#soup.flawRate 100
#run 1000
#puts stderr flaw
#assert \[genebank.rambankSize\]>1
#soup.clear
#assert \[genebank.rambankSize\]==0
soup.flawRate 10000000

# insert the anscestor, and then run the simulator some with various flaws
injectOrg 0080aaa
injectOrg 0045aaa
soup.mutRate 100
run 50
puts stderr mutate
#assert \[genebank.rambank.size\]>2
soup.clear
assert \[genebank.rambankSize\]==0
soup.mutRate 10000000

# insert the anscestor, and then run the simulator some with various flaws
injectOrg 0080aaa
soup.copyFlawRate 10
run 100000
puts stderr copy
assert \[genebank.rambankSize\]>1
soup.clear
assert \[genebank.rambankSize\]==0
