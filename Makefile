# Autor: Henrique de Carvalho <henriquecarvalho@usp.br>

###################################
######         FLAGS         ######
###################################

CFLAG := -Wall -Werror -pedantic

###################################
######      DIRECTORIES      ######
###################################

TARGET   := broker

SRCDIR   := src
INCDIR   := include
BINDIR   := bin
OBJDIR   := obj
DEPDIR   := dep

BIN := $(BINDIR)/$(TARGET)
SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(wildcard $(OBJDIR)/%.o)
INC := $(wildcard $(INCDIR)/*.h)
DEP := $(wildcard $(DEPDIR)/*.d)

TESTSRCDIR   := tests/fonte
TESTINCDIR   := tests/include
TESTBINDIR   := tests/bin
TESTOBJDIR   := tests/obj
TESTDEPDIR   := tests/dep

TESTBIN := $(wildcard $(TESTBINDIR))
TESTSRC := $(wildcard $(TESTSRCDIR)/*.c)
TESTOBJ := $(wildcard $(TESTOBJDIR)/%.o)
TESTINC := $(wildcard $(TESTINCDIR)/*.h)
TESTDEP := $(wildcard $(TESTDEPDIR)/*.d)

###################################
######       COMMANDS        ######
###################################

CC    := gcc
MKDIR := mkdir -p
RMDIR := rm -rf

###################################
######      COMPILATION      ######
###################################

all: clean | $(BIN)

target: $(OBJDIR)
	$(CC) -I $(INCDIR) -c $(CFLAGS) $(ARGS) -o $(OBJDIR)/$(ARGS)

$(BIN): $(OBJ) | $(BINDIR)
	$(CC) -I $(INCDIR) $(CFLAGS) $(SRC) -o $(BIN)  # @$ is the name of the target

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(INCDIR)
	$(CC) -I $(INCDIR) -c $(CFLAGS) -o $@

$(BINDIR) $(OBJDIR):
	$(MKDIR) $@

clean:
	$(RMDIR) $(OBJDIR)
	$(RMDIR) $(BINDIR)
