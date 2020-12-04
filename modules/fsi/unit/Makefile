###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
# MODULE_DIR       - Location of the MOOSE modules directory
# FRAMEWORK_DIR    - Location of the MOOSE framework
#
###############################################################################
THIS_MODULE_DIR    ?= $(shell dirname `pwd`)
MODULE_DIR         ?= $(shell dirname $(THIS_MODULE_DIR))
MOOSE_DIR          ?= $(shell dirname $(MODULE_DIR))
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
ADDITIONAL_CPPFLAGS += -Wall -Wextra
###############################################################################

# Extra stuff for GTEST
ADDITIONAL_INCLUDES	:= -isystem $(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_INCLUDES	:= -isystem $(MOOSE_DIR)/unit/include
ADDITIONAL_LIBS 	:= $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
<<<<<<< HEAD:modules/phase_field/unit/Makefile
PHASE_FIELD       := yes
=======
FSI := yes
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/unit/Makefile
include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

APPLICATION_DIR  := $(shell pwd)
<<<<<<< HEAD:modules/phase_field/unit/Makefile
APPLICATION_NAME := phase_field-unit
=======
APPLICATION_NAME := fsi-unit
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/unit/Makefile
BUILD_EXEC       := yes
app_BASE_DIR     :=
DEP_APPS    ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include $(FRAMEWORK_DIR)/app.mk

# Find all the MOOSE unit test source files and include their dependencies.
<<<<<<< HEAD:modules/phase_field/unit/Makefile
phase_field_unit_srcfiles := $(shell find $(APPLICATION_DIR)/src -name "*.C")
phase_field_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(phase_field_unit_srcfiles))
-include $(phase_field_unit_deps)
=======
fsi_unit_srcfiles := $(shell find $(APPLICATION_DIR)/src -name "*.C")
fsi_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(fsi_unit_srcfiles))
-include $(fsi_unit_deps)
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/unit/Makefile

###############################################################################
# Additional special case targets should be added here
