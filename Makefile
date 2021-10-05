
all : MiniSGBD Tests

windows : MiniSGBD win_clean
unix : clean


MiniSGBD: bin main.o DiskManager.o FileList.o BufferManager.o Frame.o 
	gcc -o bin/MiniSGBD bin/DiskManager.o bin/FileList.o bin/BufferManager.o bin/Frame.o bin/main.o

Tests: bin Tests.o DiskManager.o FileList.o BufferManager.o Frame.o
	gcc -o bin/Tests bin/DiskManager.o bin/FileList.o bin/BufferManager.o bin/Frame.o bin/Tests.o

bin:
	- mkdir bin

main.o:
	gcc -o bin/main.o -c CODE/main.c

DiskManager.o:
	gcc -o bin/DiskManager.o -c CODE/DiskManager/DiskManager.c

BufferManager.o:
	gcc -o bin/BufferManager.o -c CODE/BufferManager/BufferManager.c
	
Frame.o:
	gcc -o bin/Frame.o -c CODE/BufferManager/Frame.c

FileList.o:
	gcc -o bin/FileList.o -c CODE/DiskManager/FileList.c

Tests.o:
	gcc -o bin/Tests.o -c CODE/Tests/Tests.c
	
clean:
	rm -f bin/*.o

win_clean:
	del /F bin/*.o
