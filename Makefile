all:
	@gcc server.c -std=c89 -Wall -Ofast -o xeric

debug:
	gcc server.c -std=c89 -Wall -Werror -O0 -o xeric
	@echo

small:
	@gcc server.c -std=c89 -Wall -Os -o xeric