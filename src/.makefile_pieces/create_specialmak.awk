BEGIN {
	useline = 0;
}

(NF == 0) {
	useline = 0;
}

(useline) {
	filename_stem = substr($1,1,rindex($1,".")-1);
    print filename_stem ".virt.hm: " $1;
    print filename_stem ".autoheader.hm: " $1;
    print filename_stem ".priVar.hm: " $1;
    print filename_stem ".includes.hm: " $1;
	print  "\t@echo [--3a-] Generating \\*.hm files for \\\"" $1 "\\\"";
	print  "\t@awk -f .makefile_pieces/makehm.awk " $1;
	print "";
}

/^SPECCASES / {
	useline = 1;
}

function rindex(str,c)
{
    return match(str,"\\" c "[^" c "]*$")? RSTART : 0
}
