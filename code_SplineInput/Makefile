#-----------------------------------------------------------------------------

PROGRAM = AliSkimmedDataAnalysisMaker

HDRS = V0FlatAna.h TrackFlatAna.h AliSkimmedDataAnalysisMaker.h\
 
OBJS = V0FlatAna.o TrackFlatAna.o AliSkimmedDataAnalysisMaker.o\
        dict_$(PROGRAM).o

#-----------------------------------------------------------------------------
ObjSuf = o
SrcSuf = cxx
ExeSuf = exe
DllSuf = so
OutPutOpt = -o

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)

# Directories
HEADRDIR = $(ALICE_PHYSICS)/include
# debug option: -g
# no exceptions: -fno-exceptions

CXX = g++
CXXFLAGS = -O3 -Wall -fPIC -g  -std=c++11
LD = g++
LDFLAGS = -O3 -Wall -g 
SOFLAGS = -shared
ARCHFLAGS =

#
CXXFLAGS += $(ROOTCFLAGS) -I$(HEADRDIR) -I$(ALICE_ROOT)/include -I$(ALICE_PHYSICS)/include

LIBS = $(ROOTLIBS)
GLIBS = $(ROOTGLIBS)


%/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c "$<" -o "$@" 

$(PROGRAM).so: $(OBJS)
	@echo "Linking $(PROGRAM).so ..."
	$(LD) $(ARCHFLAGS) $(LDFLAGS) $(SOFLAGS) $(OBJS) $(ROOTLIBS) -L$(ALICE_PHYSICS)/lib -lVMC -lMinuit -lEG -lGeom -o$(PROGRAM).so
	@echo "linking done"

dict_$(PROGRAM).cpp: $(HDRS)
	@echo "Generating dictionary ..."
	rootcint -f dict_$(PROGRAM).cpp -c -p -I$(HEADRDIR) $(HDRS) Linkdef.h 

clean:
	@rm -f $(OBJS) $(PROGRAM).so dict_*
 

#------------------------------------------------------------------------
