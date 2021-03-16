all: bot

bot: main.o gameInfo.o structs.o textline.o
	g++ main.o gameInfo.o structs.o textline.o -Wall -o bot
	rm -rf *.o

main.o: main.cpp
	g++ -c -Wall main.cpp

structs.o: structs.cpp
	g++ -c -Wall structs.cpp

textline.o: textline.cpp
	g++ -c -Wall textline.cpp

gameInfo.o: gameInfo.cpp
	g++ -c -Wall gameInfo.cpp

clean:
	rm -rf *.o 