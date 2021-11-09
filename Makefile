CC=gcc -I CODE/


all : MiniSGBD Tests

windows : MiniSGBD win_clean
unix : clean


MiniSGBD: bin main.o DiskManager.o FileList.o BufferManager.o Frame.o Record.o RelationInfo.o Catalog.o FileManager.o DBManager.o stringutil.o fileutil.o
	gcc -o bin/MiniSGBD bin/DiskManager.o bin/FileList.o bin/BufferManager.o bin/Frame.o bin/main.o bin/Catalog.o bin/DBManager.o bin/FileManager.o bin/RelationInfo.o bin/Record.o bin/stringutil.o bin/fileutil.o

Tests: bin Tests.o DiskManager.o FileList.o BufferManager.o Frame.o stringutil.o fileutil.o
	gcc -o bin/Tests bin/DiskManager.o bin/FileList.o bin/BufferManager.o bin/Frame.o bin/Tests.o bin/fileutil.o bin/stringutil.o

bin:
	- mkdir bin

main.o:
	$(CC) -o bin/main.o -c CODE/main.c

DiskManager.o:
	$(CC) -o bin/DiskManager.o -c CODE/DiskManager/DiskManager.c

BufferManager.o:
	$(CC) -o bin/BufferManager.o -c CODE/BufferManager/BufferManager.c
	
Frame.o:
	$(CC) -o bin/Frame.o -c CODE/BufferManager/Frame.c

FileList.o:
	$(CC) -o bin/FileList.o -c CODE/DiskManager/FileList.c

Tests.o:
	$(CC) -o bin/Tests.o -c CODE/Tests/Tests.c

RelationInfo.o:
	$(CC) -o bin/RelationInfo.o -c CODE/FileManager/RelationInfo.c

Catalog.o:
	$(CC) -o bin/Catalog.o -c CODE/FileManager/Catalog.c

Record.o:
	$(CC) -o bin/Record.o -c CODE/FileManager/Record.c

FileManager.o:
	$(CC) -o bin/FileManager.o -c CODE/FileManager/FileManager.c

DBManager.o:
	$(CC) -o bin/DBManager.o -c CODE/DBManager/DBManager.c

fileutil.o:
	$(CC) -o bin/fileutil.o -c CODE/util/fileutil.c

stringutil.o:
	$(CC) -o bin/stringutil.o -c CODE/util/stringutil.c

clean:
	rm -f bin/*.o

win_clean:
	del /F bin/*.o
