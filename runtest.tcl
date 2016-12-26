#!etierra
use_namespace etierra
genebank.Init genebank r
soup.tournamentMode 1
soup.setMaxCells 4
addEmptyCell
addEmptyCell
injectOrg 0058aaj
#injectOrg 0060aac
puts "interacts=[soup.interacts]"
#run 10000
runJoust
puts "tstep=[soup.tstep]"
insertResults
