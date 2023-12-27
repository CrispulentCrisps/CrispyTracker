CXXFLAGS := -std=c++11 -DCT_UNIX -I Libraries/imgui
LDFLAGS := $(shell pkg-config --libs glfw3 sndfile SDL2) $(shell sdl2-config --cflags) 

SOURCES := Source.cpp Tracker.cpp Channel.cpp Patterns.cpp SoundGenerator.cpp

ct: $(SOURCES) libimgui.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

libimgui.a: Libraries/imgui/imgui.cpp Libraries/imgui/imgui_draw.cpp Libraries/imgui/imgui_demo.cpp Libraries/imgui/imgui_tables.cpp Libraries/imgui/imgui_widgets.cpp Libraries/imgui/backends/imgui_impl_opengl3.cpp Libraries/imgui/backends/imgui_impl_glfw.cpp Libraries/ImGuiFileDialog-0.6.5/ImGuiFileDialog.cpp 
	$(CXX) $(CXXFLAGS) $(shell pkg-config --cflags glfw3) -c $^ 
	ar rcs libimgui.a imgui*.o ImGui*.o
