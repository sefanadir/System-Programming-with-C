all:
	gcc -c -pedantic-errors -Wall cat_command.c 
	gcc -o cat_exe  cat_command.o 

	gcc -c -pedantic-errors -Wall ls_command.c 
	gcc -o ls_exe  ls_command.o 

	gcc -c -pedantic-errors -Wall wc_command.c 
	gcc -o wc_exe  wc_command.o 

	gcc -c -pedantic-errors -Wall 131044031_main.c 
	gcc -o exe 131044031_main.o 

clean:
	rm *.o exe cat_exe ls_exe wc_exe
