#!etierra
use_namespace etierra
genebank.Init genebank w

foreach name [exec ls src/] {
    importGenome src/$name
}

genebank.reverseGenebank.Init genebankR w
genebank.populateReverseGenebank
