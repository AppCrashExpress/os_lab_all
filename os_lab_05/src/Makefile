LIBRARY_NAME = MD5Queue
LIBRARY_FLAGS = -Wall
BINARY_FLAGS = -Wall

.PHONY: all
all: dynamic dynamic_sys

dynamic: dynamic_lib queue.c
	gcc $(BINARY_FLAGS) -Llib queue.c -l$(LIBRARY_NAME) -o dynamic_queue

dynamic_sys: dynamic_lib queue_dyn.c
	gcc $(BINARY_FLAGS) queue_dyn.c -ldl -o dynamic_queue_sys

#No dependencies since version control needs to be performed manually
dynamic_lib:
	gcc $(LIBRARY_FLAGS) -fPIC -c $(LIBRARY_NAME).c
	gcc $(LIBRARY_FLAGS) -shared -o lib/lib$(LIBRARY_NAME).so $(LIBRARY_NAME).o 

static: static_lib queue.c
	gcc $(LIBRARY_FLAGS) -Wall queue.c -Llib/ -l$(LIBRARY_NAME) -o static_queue

static_lib: $(LIBRARY_NAME).h $(LIBRARY_NAME).c
	gcc $(LIBRARY_FLAGS) -Wall -c $(LIBRARY_NAME).c
	ar -cvq lib/lib$(LIBRARY_NAME).a $(LIBRARY_NAME).o

.PHONY: clean
clean:
	rm -f *.o dynamic_queue dynamic_queue_sys static_queue
