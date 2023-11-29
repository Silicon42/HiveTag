MAKEFLAGS += --no-builtin-rules  #black magic removal
# Note: 
# If the .d (dependency) files are manually removed the dependencies for include files will not
# get picked up unless there is a change in the .c file or until the next "make clean" is executed.
CFLAGS += -g -O0 -Wall -Wextra -Werror -Wno-shift-negative-value

MODULE_DIR := submodules/Tiny_ECC/ReedSolomon/
MODULE_SRC_DIR := $(MODULE_DIR)src/
MODULE_INC_DIR := $(MODULE_DIR)inc/
MODULE_OBJ_DIR := $(MODULE_DIR)obj/

OBJ_DIR := obj/
SRC_DIR := src/
INC_DIRS := inc/ $(MODULE_INC_DIR)
# .c files in this directory have a main function in them and are thus mutually exclusive when linking
APPS_DIR := apps/

# gets a list of all applications in the apps dir for filtering build targets
APP_LIST := $(basename $(shell find $(APPS_DIR) -type f -printf "%f\n"))

# The name of the executable to build -- it will end up in the working directory
BUILD_TARGET := $(filter $(APP_LIST),$(MAKECMDGOALS))

# Following are the search paths for prerequisites 
vpath %.h $(INC_DIRS)	# Include file search path includes the source path  -- This should also be conveyed to the complile rule
vpath %.c $(SRC_DIR) $(APPS_DIR) $(MODULE_SRC_DIR)
#vpath %.d $(OBJ_DIR)

# Add a prefix to INC_DIRS to add the "-I" compile flag 
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CPPFLAGS := $(INC_FLAGS) -MMD -MP #-MMD and -MP generates the .d files when a .c file is compiled

# Manually entering the obj targets
#OBJfiles := sub.o main.o	
#OBJECTS := $(patsubst %, $(OBJ_DIR)%, $(OBJfiles))	# Creates the object target list in the OBJ_DIR folder

#Automatically scanning for source files and creating .o targets from .c files found
# create object target list by searching for .c files in the source path and then  switch .c with .o and substitute the obj path for the src 
TARGET_SRC := $(APPS_DIR)$(BUILD_TARGET).c
TARGET_OBJ := $(subst $(APPS_DIR),$(OBJ_DIR),$(TARGET_SRC:.c=.o))

APP_SOURCES := $(shell find $(APPS_DIR) -name "*.c")
APP_OBJECTS := $(subst $(APPS_DIR),$(OBJ_DIR),$(APP_SOURCES:.c=.o))

SOURCES := $(shell find $(SRC_DIR) -name "*.c")
OBJECTS := $(subst $(SRC_DIR),$(OBJ_DIR),$(SOURCES:.c=.o))

# Hardcoded because these are the only two ever needed by this project and that shouldn't change anytime soon
MODULE_OBJECTS := $(addprefix $(MODULE_OBJ_DIR),gf16.o rs_gf16.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJECTS:.o=.d) $(APP_OBJECTS:.o=.d)

#####   First Target -- This is what gets built by default #####
.PHONY: all
all: $(OBJECTS) $(APP_OBJECTS)
	$(MAKE) all -C $(MODULE_DIR)
	@echo compiled all objects

# VScode Makefile tools is stupid and doesn't allow manually adding targets, instead it only allows
#  explicitly specified targets so these are dummy rules for the applications.
gen_chk_sym_shifter:
gen_basis_vectors:
gen_cw_to_id_k3_k4_LUT:
gen_tags:
rs_codeword_test:
test_ids:


$(BUILD_TARGET) :$(OBJECTS) $(TARGET_OBJ) $(MODULE_OBJECTS)	# This will drive the creation of the .o files in the OBJdir for the prerequisites 
	@echo "$@ link rule"
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ $(LOADLIBES) $(LDLIBS) -o $@

# the compile rule for the prerequisits of the final target --
$(OBJ_DIR)%.o : %.c				# pattern rule picks up the .c as a pre-req for a .o
	@#echo "MY pattern rule .c to .o via OBJ_DIR"
	@mkdir -p '$(@D)'
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(MODULE_OBJ_DIR)%.o : %.c
	@echo "making $(subst $(MODULE_DIR),./,$@)"
	$(MAKE) $(subst $(MODULE_DIR),./,$@) -C $(MODULE_DIR)

.PHONY: clean
clean:
	$(MAKE) clean -C $(MODULE_DIR)
	@echo cleaning $(OBJ_DIR)
	@rm -rf $(OBJ_DIR)			# removing all of the OBJS forces a complete re-compile / re-link
	@rm -f *.exe				# just for good measure remove the exe as well 

# see automatic generation via -MMD -MP and include $(DEPS) below
-include $(DEPS)


# some built in default variables from make -- as displayed by make -p
#LINK.c =  														% : %c
#$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)  		$^ $(LOADLIBES) $(LDLIBS) -o $@

#LINK.o =  														% : %.o
# $(CC) $(LDFLAGS) $(TARGET_ARCH) 								$^ $(LOADLIBES) $(LDLIBS) -o $@

#COMPILE.c =  													%.o : %.c  
#$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c 					$(OUTPUT_OPTION) $<


# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.

#some example dependencies 
# The internal dependencies for the various object targets  No need to specify the .c file here, it gets picked up by the compile pattern rule
# This should get generated by the compiler and then included as a seperate file -- Note that the $(OBJ_DIR) path has to be pre-pended 
#$(OBJ_DIR)main.o:min.h max.h		# no need to specify the .c file, it gets picked up by the %.o : %.c pattern rule
#$(OBJ_DIR)main.o:defs.h			# add one more dependency -- this one is found in the second vpath entry for .h
#$(OBJ_DIR)sub.o: defs.h			# this indicates that def.h is a prerequisit for sub.o file only
# example of multiple targets depending on the same pre-req
#$(OBJ_DIR)main.o $(OBJ_DIR)/sub.o : defs.h	# this indicates that def.h is a prerequisit for both .o files 

