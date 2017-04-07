#!etierra
use_namespace etierra
genebank.Init genebank r
soup.tournamentMode 1
soup.setMaxCells 4
injectOrg 0187abb
addEmptyCell
injectOrg 0159aae
addEmptyCell
#puts "interacts=[soup.interacts]"
#soup.cells.@elem 2
#etierra.soup.cells(2).cpu.active 0
runJoust
puts "tstep=[soup.tstep]"
insertResults
