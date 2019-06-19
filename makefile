ROOTDIR=..
IMGUI=$(ROOTDIR)/imgui
IMGUIBIN=$(IMGUI)/bin
IMGUISOURCES=$(wildcard $(IMGUI)/*.cpp)
IMGUISOURCES += $(IMGUI)/examples/imgui_impl_opengl3.cpp $(IMGUI)/examples/imgui_impl_glfw.cpp
IMGUIOBJS=$(patsubst %.cpp, %.o, $(IMGUISOURCES))
SIMAVR=$(ROOTDIR)/simavr/simavr
SIMAVRBIN=$(SIMAVR)/bin
SIMAVRSOURCES=$(wildcard $(SIMAVR)/cores/*.c) $(wildcard $(SIMAVR)/sim/*.c)
SIMAVROBJS=$(patsubst %.c, %.o, $(SIMAVRSOURCES))
SIMGUI=$(ROOTDIR)/simgui
SIMGUISRC=$(SIMGUI)/src
SIMGUIINCLUDE=$(SIMGUI)/include
SIMGUIBIN=$(SIMGUI)/bin
SIMGUIBUILD=$(SIMGUI)/build
SIMGUISOURCES=$(wildcard $(SIMGUISRC)/*.cpp)
SIMGUIOBJS=$(patsubst src/%.cpp, bin/%.o, $(SIMGUISOURCES))
GL3W=$(ROOTDIR)/gl3w
GL3WBIN=$(GL3W)/bin
TARGET=$(SIMGUIBUILD)/simgui
OBJS=$(SIMAVRBIN)/simavr.o $(IMGUIBIN)/imgui.o $(SIMGUIBIN)/simgui.o $(GL3WBIN)/gl3w.o
LIBS += -lGL -lelf `pkg-config --static --libs glfw3`
CXXFLAGS += -I /usr/include -I$(SIMGUIINCLUDE) -I$(SIMAVR) -I$(IMGUI) -I$(SIMAVR)/sim -I$(SIMAVR)/sim/avr -I$(SIMAVR)/cores -I$(SIMAVR)/cores/avr
LDFLAGS += -L/usr/lib/gcc/x86_64-linux-gnu/7.4.0



$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)
##gl3w
$(GL3WBIN)/gl3w.o: $(GL3W)/gl3w.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

##SIMAVR
$(SIMAVRBIN)/simavr.o: $(SIMAVROBJS)
	$(LD) $(LDFLAGS) -r $(SIMAVROBJS) -o $@ 

$(SIMAVR)/cores/%.o: $(SIMAVR)/cores/%.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(SIMAVR)/sim/%.o: $(SIMAVR)/sim/%.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

##IMGUI
$(IMGUIBIN)/imgui.o: $(IMGUIOBJS)
	$(LD) $(LDFLAGS) -r $(IMGUIOBJS) -o $@ 

$(IMGUIBIN)/%.o: $(IMGUI)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

##SIMGUI

$(SIMGUIBIN)/simgui.o: $(SIMGUIOBJS)
	$(LD) $(LDFLAGS) -r $(SIMGUIOBJS) -o $@

$(SIMGUIBIN)/%.o: $(SIMGUISRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean: cleangl3w cleansimavr cleanimgui cleansimgui

cleangl3w:
	mv $(GL3WBIN)/gl3w.o ~/.Trash/

cleansimavr:
	mv $(SIMAVROBJS) ~/.Trash

cleanimgui:
	mv $(IMGUIOBJS) ~/.Trash

cleansimgui:
	mv $(SIMGUIOBJS) ~/.Trash