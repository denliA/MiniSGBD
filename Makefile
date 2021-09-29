
all : MiniSGBD Tests

windows : MiniSGBD win_clean
unix : clean

MiniSGBD: main.o DiskManager.o FileList.o bin
	gcc -o bin/MiniSGBD bin/DiskManager.o bin/FileList.o bin/main.o

Tests: Tests.o DiskManager.o FileList.o bin
	gcc -o bin/Tests bin/DiskManager.o bin/FileList.o bin/Tests.o

bin:
	- mkdir bin

main.o:
	gcc -o bin/main.o -c CODE/main.c

DiskManager.o:
	gcc -o bin/DiskManager.o -c CODE/DiskManager/DiskManager.c

FileList.o:
	gcc -o bin/FileList.o -c CODE/DiskManager/FileList.c

Tests.o:
	gcc -o bin/Tests.o -c CODE/Tests/Tests.c
	
clean:
	rm -f bin/*.o

win_clean:
	del /F bin/*.o
