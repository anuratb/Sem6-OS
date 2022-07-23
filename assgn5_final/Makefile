
test: libmemlab.a
	g++  $(prog) -lpthread -L. -lmemlab
libmemlab.a: memlab.o
	ar -rcs libmemlab.a memlab.o
memlab.o : memlab.cpp
	g++ -c memlab.cpp 
clear:
	rm -rf *.o *.a ./a.out