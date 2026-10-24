all: vn/vn

vn/vn: vn/vn.c
	gcc -Wall -o vn/vn vn/vn.c

clean:
	rm -f vn/vn
