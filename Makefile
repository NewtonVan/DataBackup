CXX = g++
CXX_INCLUDE_FLAGS = -Iinc 
LIBS = -lboost_system -ljsoncpp -lpthread -lcrypto
CXXFLAGS = -std=c++11 $(CXX_INCLUDE_FLAGS) -w

CPP_SRC_FILES = $(shell find .  -name "*.cpp" -not -path "./test/*")
CPP_OBJ_FILES = $(patsubst %.cpp, %.o, $(CPP_SRC_FILES))
CPP_DPT_FILES = $(patsubst %.cpp, %.d, $(CPP_SRC_FILES))

DST = ./build/dbp

$(DST) : $(CPP_OBJ_FILES)
	@$(CXX) -o $@ $^ $(CXXFLAGS)  $(LIBS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
%.d : %.cpp
	@set -e; \
	rm -f $@; \
	$(CXX) -MM $< $(CXX_INCLUDE_FLAGS) > $@.$$$$.dtmp; \
	sed 's,.*\.o\:,$*\.o $*\.d\:,g' < $@.$$$$.dtmp > $@; \
	rm -f $@.$$$$.dtmp

-include $(CPP_DPT_FILES)

.PHONY: clean run
clean :
	rm -f $(DST)
	rm -f $(CPP_OBJ_FILES) $(CPP_DPT_FILES)
	rm -f $(shell find . -name "*.dtmp")	

run :
	make
	$(DST) test 2>> errlog 
	
runAll :
	make
	$(DST) test 2>> errlog &
	cd front_end && npm start