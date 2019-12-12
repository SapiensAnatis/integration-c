all:
	gcc project.c rpn.c stack.c -lm -lpcre2-8 -o project.out && ./project.out
