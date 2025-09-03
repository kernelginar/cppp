all:
	@echo "Building src/cppp.c file..."
#@gcc -o build/cppp src/cppp.c -O3 -D_FILE_OFFSET_BITS=64 -lcrypto src/include/print_warns.c
	@gcc -o build/cppp src/cppp.c -O3 -D_FILE_OFFSET_BITS=64 -lcrypto src/include/print_warns.c -fsanitize=address -g

clean:
	rm -rf build/cppp
