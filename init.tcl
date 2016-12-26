#!etierra
use_namespace etierra
genebank.Init genebank w
foreach src [exec ls src] {
    puts "adding $src"
    importGenome src/$src
} 
genebank.reverseGenebank.Init genebankR w
genebank.populateReverseGenebank
genebank.reverseGenebank.close
genebank.close
