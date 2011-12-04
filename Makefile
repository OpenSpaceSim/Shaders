CC=clang++
CFLAGS+=-g
LDFLAGS+=-L. -lassimp -lGL -lGLU -lglut -lGLEW
OFILES=main.o shader.o util.o
INCLUDEFLAGS = -I./OpenAssetImport/include 

%.o: %.cpp $(DEPS)
	$(CC) $(INCLUDEFLAGS) -c -o $@ $< $(CFLAGS)

openSpaceSim : $(OFILES)
	$(CC) -o openSpaceSim $(OFILES) $(LDFLAGS)
	
go:
	$(CC) -o openSpaceSim $(OFILES) $(LDFLAGS)
	./openSpaceSim
	
.PHONY: clean

clean:
	rm -f *.o *~ openSpaceSim
