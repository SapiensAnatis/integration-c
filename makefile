all:
	gcc project.c tokenize.c stack.c -lm -lpcre2-8 -o project.out && ./project.out
