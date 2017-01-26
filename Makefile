default: simpsh

simpsh: simpsh.c
	gcc -o simpsh -g simpsh.c

check: simpsh
	@echo off ; \
	folder=804608241makechecktest ; \
	mkdir $${folder} ; \
	cp simpsh $${folder}/simpsh ; \
	cd $${folder} ; \
	str="This should be printed to output." ; \
	echo $${str} > in.txt ; \
	empty="" ; \
	echo $${empty} > out.txt ; \
	echo $${empty} > err.txt ; \
	remove="rm -r 804608241makechecktest" ; \
	./simpsh --rdonly=in.txt --wronly=out.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -eq 0 ] ; \
	then \
	echo "test 1 passed." ; \
	else \
	echo "test 1 failed." ; \
	rm * ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	./simpsh --rdonly=dne.txt --wronly=out.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -ne 0 ] ; \
	then \
	echo "test 2 passed." ; \
	else \
	echo "test 2 failed." ; \
	rm * ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	./simpsh --rdonly=in.txt --wronly=dne.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -ne 0 ] ; \
	then \
	echo "test 3 passed." ; \
	else \
	echo "test 3 failed." ; \
	rm * ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	./simpsh --rdonly --wronly=out.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -ne 0 ] ; \
	then \
	echo "test 4 passed." ; \
	else \
	echo "test 4 failed." ; \
	rm * ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	echo "ALL TESTS PASSED." ; \
	rm * ; \
	cd .. ; \
	$${remove}

clean: 
	rm -f simpsh lab1-yamatosasaki.tar.gz

dist: simpsh.c README Makefile
	mkdir lab1-yamatosasaki ; \
	cp simpsh.c lab1-yamatosasaki/simpsh.c ; \
	cp README lab1-yamatosasaki/README ; \
	cp Makefile lab1-yamatosasaki/Makefile ; \
	tar -czvf lab1-yamatosasaki.tar.gz lab1-yamatosasaki ; \
	rm -r lab1-yamatosasaki
