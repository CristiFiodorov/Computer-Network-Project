
all:
	g++ ServerFolder/CastDocServ.cpp ClientFolder/includes.cpp -o ServerFolder/CastDocServ
clean:
	rm -f ServerFolder/CastDocServ
