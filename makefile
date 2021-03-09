all:	 TestMessage TestRedirection TestLectureEcriture Terminal CreationDatabase

message.o: message.c message.h
	gcc -Wall -c message.c

alea.o: alea.h alea.c
	gcc -Wall -c alea.c

TestMessage: message.o alea.o TestMessage.c
	gcc -Wall TestMessage.c message.o alea.o -o  TestMessage

TestRedirection: TestRedirection.c
	gcc -Wall TestRedirection.c -o  TestRedirection

lectureEcriture.o: lectureEcriture.c lectureEcriture.h
	gcc -c -Wall lectureEcriture.c

TestLectureEcriture: lectureEcriture.o TestLectureEcriture.c
	gcc lectureEcriture.o TestLectureEcriture.c -o TestLectureEcriture

Terminal: message.o alea.o lectureEcriture.o terminal.c terminal.h
	gcc -Wall terminal.c message.o alea.o lectureEcriture.o -o  Terminal

CreationDatabase: alea.o lectureEcriture.o creationDatabase.c 
	gcc -Wall creationDatabase.c lectureEcriture.o alea.o  -o CreationDatabase

clean:	
	rm -f *.o *~ 

cleanall: clean
	rm TestRedirection TestMessage TestLectureEcriture Terminal
