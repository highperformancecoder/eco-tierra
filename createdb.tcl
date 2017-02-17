#!etierra
set torgs [open tierra-orgs.txt r]
set createdb [open createdb w]
etierra.torg_headers.Init torg_headers r

while {![eof $torgs]} {
    gets $torgs buf
    etierra.torg_headers.load_header $buf
    if {[etierra.torg_headers.MaxPop] > 0} {
	puts $createdb "$buf [etierra.torg_headers.origin]"
    }
}

close $createdb
