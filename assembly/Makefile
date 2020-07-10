SOURCES=$(shell find . -type f -name '*.c')
TARGET=20150614.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc  -g $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
