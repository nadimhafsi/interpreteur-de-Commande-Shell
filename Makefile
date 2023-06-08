																																						
exec: shell.o 
	gcc -o exec shell.o  
	
shell.o :shell.c  
	gcc -o shell.o -c shell.c 
clean:
	rm -rf *.o
