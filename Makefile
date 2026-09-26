# Project Name
TARGET = HiKelso

USE_DAISYSP_LGPL = 1

# Sources
CPP_SOURCES += HiKelso.cpp
CPP_SOURCES += HiKelso_Controls.cpp
CPP_SOURCES += HiKelso_State.cpp
CPP_SOURCES += FreeChord.cpp
CPP_SOURCES += FreeRoot.cpp
CPP_SOURCES += Sequencer.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

