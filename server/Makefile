

OBJS = server1.o server2.o server3.o server4.o server5.o server6.o server7.o server8.o

TARGET = server1 server2 server3 server4 server5 server6 server7 server8

RM = rm -f
CC = g++

LL = -lpthread


all:$(TARGET)

$(OBJS):%.o:%.cpp
		$(CC) -c $< -o $@

server1:server1.o
		g++ -g -o $@ server1.o

server2:server2.o
		g++ -g -o $@ server2.o	

server3:server3.o
		g++ -g -o $@ server3.o	$(LL)

server4:server4.o
		g++ -g -o $@ server4.o

server5:server5.o
		g++ -g -o $@ server5.o $(LL)

server6:server6.o
		g++ -g -o $@ server6.o $(LL)

server7:server7.o
		g++ -g -o $@ server7.o $(LL)

server8:server8.o
		g++ -g -o $@ server8.o $(LL)

clean:
		-$(RM) $(OBJS) $(TARGET)
