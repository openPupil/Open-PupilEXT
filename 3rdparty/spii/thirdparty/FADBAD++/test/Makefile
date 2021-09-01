CC = g++ -g
CFLAGS = -I.. -D_DEBUG
OBJ = TestFADBAD.o TestTAD.o TestUDT.o TestAll.o

.cpp.o:
	$(CC) $(CFLAGS) -c $<

TestAll : $(OBJ)
	$(CC) -o $@ $(OBJ)

all: TestAll

clean:
	-rm *.o TestAll
