# ndstool vars
TEXT1 := libndsgfx Test
TEXT2 := Test for libndsgfx
TEXT3 :=
ICON := $(CURDIR)/../logo.bmp

.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error DEVKITARM environment variable not set)
endif

include $(DEVKITARM)/ds_rules

# dirs
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	src
DATA		:=	data # binary data to pass through bin2o
INCLUDES	:=	include

# options for code generation
ARCH		:= -mthumb -mthumb-interwork

CFLAGS		:= -g -Wall -O2 -march=armv5te -mtune=arm946e-s -fomit-frame-pointer \
	-ffast-math $(ARCH) $(INCLUDE) -DARM9
CXXFLAGS 	:= $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS 	:= -g $(ARCH)
LDFLAGS 	= -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -L../lib

# extra libs
LIBS := -lfat -lfreetype -lnds9
 
 
# list of directories containing libraries, this must be the top level containing
# include and lib
LIBDIRS	:=	$(LIBNDS)

ifneq ($(BUILDDIR), $(CURDIR))
 
# harvest files
export OUTPUT	:=	$(CURDIR)/$(TARGET)
 
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
					$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
BMPFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.bmp)))

# use CXX for linking C++ projects, CC for standard C
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(BMPFILES:.bmp=.o) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean
 
# build
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make BUILDDIR=`cd $(BUILD) && pwd` --no-print-directory -C $(BUILD) \
		-f $(CURDIR)/Makefile
 
# clean up
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).arm9 \
		$(TARGET).ds.gba 
 
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
# main targets
$(OUTPUT).nds	: 	$(OUTPUT).arm9
	@ndstool -c $@ -9 $< -b $(ICON) "$(TEXT1);$(TEXT2);$(TEXT3)"
$(OUTPUT).arm9	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

# link .bin/.raw files
%.bin.o	:	%.bin
	@echo $(notdir $<)
	@$(bin2o)

%.raw.o	:	%.raw
	@echo $(notdir $<)
	@$(bin2o)
 
-include $(DEPENDS)
 
endif
