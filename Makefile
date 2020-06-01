SANE_INCLUDE=/home/yusq/kylin-sane-test/include/
SANE_LIB=-lsane
SOURCE=main.cpp kylin_sane.cpp
TARGET=kylinSane

$(TARGET): $(SOURCE)
	g++ -o kylinSane -I$(SANE_INCLUDE) $(SOURCE) $(SANE_LIB)

clean:
	rm -f $(TARGET)
