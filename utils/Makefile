.SUFFIXES: .cc .o 

ALL = create_delay_function

create_delay_function: create_delay_function.o
	$(CXX) -o $@ $< -L/jop54_0/kruithof/share/lib -lgsl -lgslcblas -lm 


all: $(ALL)

clean:
	rm -f $(ALL) create_delay_function.o
