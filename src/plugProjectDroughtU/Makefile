DROUGHT_FILES:=\
	$(BUILD_DIR)/src/plugProjectDroughtU/GoHereMap.o\
	$(BUILD_DIR)/src/plugProjectDroughtU/GoHereNavi.o\
	$(BUILD_DIR)/src/plugProjectDroughtU/FastPathfinder.o\

DEPENDS += $(DROUGHT_FILES:.o=.d)

$(BUILD_DIR)/src/plugProjectDroughtU/plugProjectDroughtU.a: $(DROUGHT_FILES)
	@echo Linking... $@
	$(QUIET) mkdir -p $(dir $@)
	@echo $(DROUGHT_FILES) > build/DROUGHT_ofiles
	$(QUIET) $(LD) -library $(LIBRARY_LDFLAGS) -o $@ -lcf ldscript.lcf @build/DROUGHT_ofiles
