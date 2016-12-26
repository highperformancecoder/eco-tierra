#!etierra
use_namespace etierra
genebank.Init genebank r
genebank.max_elem 1000

neutrals.orgnms [genebank.orgList]
neutrals.resultDb.Init results r
neutrals.resultDb.max_elem 1000
neutrals.resultDbIdx.Init resultIdx r
neutrals.resultDbIdx.max_elem 1000

puts "removing neutrals"
neutrals.rem_neutrals

set uniqueOrgs [open uniqueOrgs w]

foreach org [lsort [neutrals.orgnms]] {
    puts $uniqueOrgs $org
}
close $uniqueOrgs
