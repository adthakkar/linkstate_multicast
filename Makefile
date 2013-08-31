all:node controller

node:node.o
			g++ node.o -o node

controller:controller.o
						g++ controller.o -o controller

node.o:node.cc
				g++ -c node.cc node.h
  
controller.o:controller.cc
							g++ -c controller.cc node.h
  
clean:
			rm -f *.o *.h.gch node controller
			touch *
