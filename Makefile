SANE_INCLUDE=/home/yusq/kylin-sane-test/include/
SANE_LIB=-lsane
SOURCE=main.c kylin_sane.c
TARGET=kylinSane

$(TARGET): $(SOURCE)
	gcc -o kylinSane -I$(SANE_INCLUDE) $(SOURCE) $(SANE_LIB)

clean:
	rm -f $(TARGET)
