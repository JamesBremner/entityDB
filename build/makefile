#source file search paths
VPATH = ./src ../raven-set  ../sqlite

INC = -I./include \
	-I../windex/include \
	-I../raven-set \
	-I../boost/boost1_72 

LIBS = \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs \
	-lws2_32 -lwsock32 \
	#-mwindows

static_link_flags = -static-libstdc++ -static-libgcc -static

# folder for .o files
ODIR=build/obj

OPT = -g

# sources
_SRC = \
	family.o \
	entitydb.o raven_sqlite.o sqlite3.o
SRC = $(patsubst %,$(ODIR)/%,$(_SRC))

$(ODIR)/sqlite3.o: sqlite3.c
	gcc	 -c -o $@ $<

$(ODIR)/%.o: %.cpp entitydb.h
	g++ $(OPT)  -std=c++17 -D_mingw_  -D_WIN32 \
	-c -o $@ $< $(INC)

family: $(SRC)
	g++ $(OPT) \
	-o bin/family.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q build\obj

