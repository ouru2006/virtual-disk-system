.phony all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc diskinfo.c  -lreadline -ltermcap -o diskinfo

disklist: disklist.c
	gcc disklist.c  -lreadline -ltermcap -o disklist

diskget: diskget.c
	gcc diskget.c  -lreadline -ltermcap -o diskget

diskput: diskput.c
	gcc diskput.c  -lreadline -ltermcap -o diskput


.PHONY clean:
clean:
	-rm -rf *.o *.exe diskinfo disklist diskget diskput
