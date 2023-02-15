
all:
	g++ ServerFolder/CastDocServ.cpp CastDoc/includes.cpp -o ServerFolder/CastDocServ
clean:
	rm -f ServerFolder/CastDocServ
