all:
	gcc project.c tokenize.c stack.c shunting.c token.c -lm -lpcre2-8 -g -o project.out && ./project.out

