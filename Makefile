
OBJS = client1.o

TARGET = client1

RM = rm -f
CC = g++

all:$(TARGET)

$(OBJS):%.o:%.cpp
		$(CC) -c $< -o $@

client1:client1.o
		g++ -g -o $@ client1.o

clean:
		-$(RM) $(OBJS) $(TARGET)
