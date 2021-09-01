CC = g++
CFLAGS = -I..

.cpp.o:
	$(CC) $(CFLAGS) -c $<

all: ExampleBAD ExampleBADFAD1 ExampleBADFAD2 ExampleBADFAD3 ExampleBADFAD4 \
     ExampleFAD ExampleTAD1    ExampleTAD2    ExampleTAD3    MonteCarlo

ExampleBAD : ExampleBAD.o
	$(CC) -o $@ $@.o

ExampleBADFAD1 : ExampleBADFAD1.o
	$(CC) -o $@ $@.o

ExampleBADFAD2 : ExampleBADFAD2.o
	$(CC) -o $@ $@.o

ExampleBADFAD3 : ExampleBADFAD3.o
	$(CC) -o $@ $@.o

ExampleBADFAD4 : ExampleBADFAD4.o
	$(CC) -o $@ $@.o

ExampleFAD : ExampleFAD.o
	$(CC) -o $@ $@.o

ExampleTAD1 : ExampleTAD1.o
	$(CC) -o $@ $@.o

ExampleTAD2 : ExampleTAD2.o
	$(CC) -o $@ $@.o

ExampleTAD3 : ExampleTAD3.o
	$(CC) -o $@ $@.o

MonteCarlo : MonteCarlo.o
	$(CC) -o $@ $@.o

clean:
	-rm *.o ExampleBAD ExampleBADFAD1 ExampleBADFAD2 ExampleBADFAD3 \
        ExampleBADFAD4 ExampleFAD ExampleTAD1 ExampleTAD2 ExampleTAD3 \
        MonteCarlo
