#!etierra
use_namespace etierra
genebank.Init genebank r
soup.tournamentMode 1
soup.setMaxCells 4
injectOrg 0079aay
addEmptyCell
injectOrg 0238aav
addEmptyCell
#
#addEmptyCell
puts "interacts=[soup.interacts]"
#run 10000
runJoust
puts "tstep=[soup.tstep]"
insertResults
