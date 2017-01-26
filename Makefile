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
	remove="rm -rf 804608241makechecktest" ; \
	./simpsh --rdonly=in.txt --wronly=out.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -eq 0 ] ; \
	then \
	echo "test 1 passed." ; \
	else \
	echo "test 1 failed." ; \
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
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	./simpsh --rdwr=in.txt --rdwr=out.txt --wronly=err.txt --command 0 1 2 cat ; \
	if [ $$? -eq 0 ] ; \
	then \
	echo "test 5 passed." ; \
	else \
	echo "test 5 failed." ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	./simpsh --rdonly=in.txt --pipe --trunc --wronly=out.txt --wronly=err.txt --command 0 2 4 cat --command 1 3 4 cat ; \
	diff in.txt out.txt ; \
	if [ $$? -eq 0 ] ; \
	then \
	echo "test 6 passed." ; \
	else \
	echo "test 6 failed." ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
./simpsh --rdonly=in.txt --trunc --wronly=out.txt --trunc --wronly=err.txt --creat --wronly out2.txt --command 0 1 2 cat --abort --command 0 3 2 cat ; \
	if [ ! -s out2.txt ] ; \
	then \
	echo "test 7 passed." ; \
	else \
	echo "test 7 failed." ; \
	cd .. ; \
	$${remove} ; \
	exit 1 ; \
	fi ; \
	echo "ALL TESTS PASSED." ; \
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
