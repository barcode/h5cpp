# Copyright (c) 2017 vargaconsulting, Toronto,ON Canada
# Author: Varga, Steven <steven@vargaconsulting.ca>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this  software  and associated documentation files (the "Software"), to deal in
# the Software  without   restriction, including without limitation the rights to
# use, copy, modify, merge,  publish,  distribute, sublicense, and/or sell copies
# of the Software, and to  permit persons to whom the Software is furnished to do
# so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE  SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT  SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY  CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
# IN  AN  ACTION  OF  CONTRACT, TORT OR  OTHERWISE, ARISING  FROM,  OUT  OF  OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

apps = harness

CXXFLAGS = -I../../ -std=c++17
LIBS =  -lprofiler -lboost_program_options -lglog -lhdf5 -lhdf5_hl -lz -ldl -lm  -lpthread

%.o : $(SRC_DIR)/%.cpp 
	$(CXX) -$(INCLUDES) -o $@  $(CPPFLAGS) $(CXXFLAGS) -c $^

all: $(apps)

harness: harness.o 
	$(CXX) $^ $(LIBS) -o $@	

test:  test-rowvec-write

test-rowvec-write: harness
	#grid-row: size  grdi-col: iteration
	./harness --test-case arma-rowvec-write --grid-row 1000000 --grid-col 10 



clean:
	@$(RM) *.o *.h5 *.callgrind *.prof $(apps)

.PHONY: test

