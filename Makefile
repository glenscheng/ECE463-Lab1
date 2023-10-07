CC = gcc

CFLAGS = -std=gnu99 -g -O3

default: all

all: http_client

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%: %.o
	$(CC) -o $@ $^

.PRECIOUS: %.o

.PHONY: default all

t_404:
	./http_client www.gnu.org 80 /software/make/manual/make.htmlll

testall: t_make.html t_index.html t_make.html.gz t_make.html_node.tar.gz t_make.info.tar.gz t_make.txt t_make.txt.gz t_make.dvi.gz t_make.pdf t_make.texi.tar.gz

t_make.html:
	./http_client www.gnu.org 80 /software/make/manual/make.html
	diff make.html expected/make.html

t_index.html:
	./http_client www.gnu.org 80 /software/make/manual/html_node/index.html
	diff index.html expected/index.html

t_make.html.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.html.gz
	diff make.html.gz expected/make.html.gz

t_make.html_node.tar.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.html_node.tar.gz
	diff make.html_node.tar.gz expected/make.html_node.tar.gz

t_make.info.tar.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.info.tar.gz
	diff make.info.tar.gz expected/make.info.tar.gz

t_make.txt:
	./http_client www.gnu.org 80 /software/make/manual/make.txt
	diff make.txt expected/make.txt

t_make.txt.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.txt.gz
	diff make.txt.gz expected/make.txt.gz

t_make.dvi.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.dvi.gz
	diff make.dvi.gz expected/make.dvi.gz

t_make.pdf:
	./http_client www.gnu.org 80 /software/make/manual/make.pdf
	diff make.pdf expected/make.pdf

t_make.texi.tar.gz:
	./http_client www.gnu.org 80 /software/make/manual/make.texi.tar.gz
	diff make.texi.tar.gz expected/make.texi.tar.gz

valall: v_make.html v_index.html v_make.html.gz v_make.html_node.tar.gz v_make.info.tar.gz v_make.txt v_make.txt.gz v_make.dvi.gz v_make.pdf v_make.texi.tar.gz

v_make.html:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.html

v_index.html:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/html_node/index.html

v_make.html.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.html.gz

v_make.html_node.tar.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.html_node.tar.gz

v_make.info.tar.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.info.tar.gz

v_make.txt:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.txt

v_make.txt.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.txt.gz

v_make.dvi.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.dvi.gz

v_make.pdf:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.pdf

v_make.texi.tar.gz:
	valgrind --leak-check=full ./http_client www.gnu.org 80 /software/make/manual/make.texi.tar.gz

clean:
	rm -f http_client; rm -r http_client.dSYM;
