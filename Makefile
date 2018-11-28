all: rainfall_seq rainfall_pt	
rainfall_seq: rainfall_seq.cpp
	g++ -O3 -std=c++11 -o rainfall_seq rainfall_seq.cpp -lpthread
rainfall_pt: rainfall_pt.cpp
	g++ -O3 -std=c++11 -o rainfall_pt rainfall_pt.cpp -lpthread
clean:
	rm rainfall_seq rainfall_pt
