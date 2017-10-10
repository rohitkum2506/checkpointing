FILE=hello
PATH_TO_LIBCKPT=.
# We use '-g3' instead of '-g'.  This extra powerful version allows
#   GDB to also print the values of macros like 'MAP_ANONYMOUS'.
CFLAGS=-g3 -O0

# See 'man pgrep' and 'man pkill' for how these commands work.
# This recipe can be used to build either of two targets: 'check' or myckpt
# The file 'myckpt' should be created when the program 'hello' ( ${FILE} )
#   receives the signal 12 (SIGUSR2).
# Notice that in this recipe, the "sleep and kill" commands run in the
#   background, and will execute only after "./${FILE}" has started executing.
check myckpt: ${FILE} myrestart
	(sleep 4 && kill -12 `pgrep -n ${FILE}` && sleep 2 && \
	 pkill -9 -n ${FILE} && echo "RESTARTING:" && make restart) &
	(sleep 10 && pkill -9 -n myrestart) &
	 ./${FILE}

restart: myrestart myckpt
	./myrestart myckpt

myrestart: myrestart.c
	gcc -static \
	  -Wl,-Ttext-segment=6400000 -Wl,-Tdata=6500000 -Wl,-Tbss=6600000 \
	  -o myrestart myrestart.c

${FILE}: ${FILE}.c libckpt.a
	gcc -static -L${PATH_TO_LIBCKPT} -lckpt -Wl,-u,myconstructor ${FILE}.c -o ${FILE}

# Bundle the .o files (there's only one, here) into an "archive" lbrary: *.a
libckpt.a: myckpt.o
	ar cr libckpt.a myckpt.o

myckpt.o: myckpt.c
	gcc -c myckpt.c

gdb: ${FILE}
	gdb ${FILE}

vi: ${FILE}.c
	vi ${FILE}.c

emacs: ${FILE}.c
	emacs ${FILE}.c

clean:
	rm -f myshell a.out *~

# 'make' views $v as a make variable and expands $v into the value of v.
# By typing $$, make will reduce it to a single '$' and pass it to the shell.
# The shell will view $dir as a shell variable and expand it.
dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
	dir=`basename $$PWD`; ls -l ../$$dir.tar.gz
