FLAGS = -Wall -Wextra -std=gnu99 -pedantic -Os

simplercon :
	gcc $(FLAGS) -o simplercon simplercon.c
clean :
	rm simplercon
