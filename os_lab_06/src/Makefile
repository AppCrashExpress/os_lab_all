.PHONY: all
all: controller worker

.PHONY: controller
controller: controller.cpp lexicon.hpp
	g++ -g controller.cpp -lzmq -o controller

.PHONY: worker
worker: worker.cpp lexicon.hpp
	g++ -g worker.cpp search.o -pthread -lzmq -o worker
