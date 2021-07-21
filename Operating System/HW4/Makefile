all:
	gcc  -g -o ddchck -pthread ddchck.c
	gcc -g -pthread -o target target.c
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
