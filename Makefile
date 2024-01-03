BUILD_DIR=./build
CXXFLAGS=-Wall -I.
CPP_SOURCES= serial.cpp  m68sys.cpp plotter.cpp main.cpp board.cpp
C_SOURCES= stepper.c m68emu/m68emu.c m68emu/m68_ops.c m68emu/m68tmr.c




OBJ = $(CPP_SOURCES:%.cpp=$(BUILD_DIR)/%.o) $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)


sp400: $(OBJ) 
	g++ $(CXXFLAGS) -o $@ $^ -lSDL2



# Include all .d files
-include $(DEP)
# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
# The -MMD flags additionaly creates a .d file with
 # the same name as the .o file.
	g++ $(CXXFLAGS) -c $< -o $@ -MMD -MP
$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D)
	gcc $(CXXFLAGS) -c $< -o $@ -MMD -MP


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
clean:
	rm -rf build sp400
