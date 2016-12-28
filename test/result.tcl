#!etierra
use_namespace etierra
soup.tournamentMode 1
soup.setMaxCells 4

genebank.Init genebank w
importGenome 0080aaa.tie
importGenome 0045aaa.tie
genebank.max_elem 1000
genebank.reverseGenebank.Init genebankR w
genebank.populateReverseGenebank

proc assert x {
    if {![expr $x]}  {
        puts stderr "assertion: $x failed"
        exit 1
    }
}

injectOrg 0080aaa
run 200
set r [soup.cells.@elem 0].result
assert \[$r.outMatches\]==0
assert \[$r.inMatches\]==0
assert "\[string equal \"\[$r.classDescription\]\" repeat\]"
assert "\[string equal \"\[$r.result\]\" 0080aaa\]"
assert \[$r.firstDiv\]==830
assert \[$r.copyTime\]==813


soup.clear
injectOrg 0080aaa
injectOrg 0045aaa
runJoust

# check 0080aaa's results
set r [soup.cells.@elem 0].result
assert \[$r.outMatches\]==0
assert \[$r.inMatches\]>0
assert "\[string equal \"\[$r.classDescription\]\" repeat\]"
assert "\[string equal \"\[$r.result\]\" 0080aaa\]"
assert \[$r.firstDiv\]==830
assert \[$r.copyTime\]==813

set inMatches0080aaa [$r.inMatches]

# now 0045aaa's
set r [soup.cells.@elem 2].result
# this assertion is invalid, as the parasite may match some daughter cells
#assert \[$r.outMatches\]==$inMatches0080aaa
assert \[$r.outMatches\]==6
assert \[$r.inMatches\]==0
assert "\[string equal \"\[$r.classDescription\]\" repeat\]"
assert "\[string equal \"\[$r.result\]\" 0045aaa\]"
assert \[$r.firstDiv\]==480
assert \[$r.copyTime\]==463

#
soup.clear
injectOrg 0045aaa
run 200
set r [soup.cells.@elem 0].result
assert \[$r.outMatches\]==0
assert \[$r.inMatches\]==0
assert "!\[string equal \"\[$r.result\]\" 0045aaa\]"


