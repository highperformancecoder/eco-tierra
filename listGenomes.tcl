#!etierra
use_namespace etierra
genebank.Init genebank r
foreach name [listKeys genebank] {
    puts $name
}
