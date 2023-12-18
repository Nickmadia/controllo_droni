# Nome del programma
TARGET = bin/drone_simulation

# Compilatore
CC = g++

# Opzioni di compilazione
CFLAGS = -Wall -std=c++20

# Directory dei file sorgente
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Directory per i file oggetto
OBJDIR = obj
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

all: $(Target)
# Comando per creare il programma eseguibile
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Compila i file oggetto
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Target per la pulizia dei file oggetto e del programma eseguibile
.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)

