
all : MiniSGBD clean

MiniSGBD: main.o DiskManager.o FileList.o bin
	gcc -o bin/MiniSGBD DiskManager.o FileList.o main.o

bin:
	- mkdir bin

main.o:
	gcc -o main.o -c CODE/main.c

DiskManager.o:
	gcc -o DiskManager.o -c CODE/DiskManager/DiskManager.c

FileList.o:
	gcc -o FileList.o -c CODE/DiskManager/FileList.c

clean:
	rm -f *.o
