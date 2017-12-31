CC = cc
COMPILER_FLAGS = -std=gnu99
#SANITIZE = -fsanitize=address
CFLAGS = $(SANITIZE) -c -g -Wall $(COMPILER_FLAGS) `pkg-config fuse --cflags`

LDFLAGS = $(SANITIZE) `pkg-config fuse --cflags --libs`

TEST = fuse_wrappers.c # Choose test file here
HEADERS=sfs_api.h bitmap.h src/geom.h src/blkio.h src/filedes.h src/inode.h src/dir.h src/fs.h
SOURCES= disk_emu.c sfs_api.c $(TEST)  bitmap.c src/blkio.c src/fs.c src/filedes.c src/inode.c src/dir.c


OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE= npfsd

all: $(EXECUTABLE)

test: clean all

$(EXECUTABLE): $(HEADERS) $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm  *.o src/*.o $(EXECUTABLE)
