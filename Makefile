xsh: xsh.c shell_helper.c
	gcc -o xsh xsh.c shell_helper.c

test: xsh
	bash test.sh