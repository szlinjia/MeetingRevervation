CXX_FLAGS = -lrt -lpthread -g -fpermissive

primarysvr:
	#g++ Sequencer.cpp -o sequencer $(CXX_FLAGS)
	g++ Replica.cpp Network.cpp -o replica $(CXX_FLAGS)
	g++ launch.cpp -o launch $(CXX_FLAGS)
	g++ PrimarySvr.cpp -o primarysvr $(CXX_FLAGS)


all: primarysvr

clean:
	rm -f primarysvr  launch replica
