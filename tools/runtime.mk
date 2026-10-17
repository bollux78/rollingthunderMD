# Always initialize TMSS, then use the project reset-state validation.
LIBGCC += -Wl,--wrap=_reset_entry
$(OUT_DIR)/sega.o: tools/cart_boot.py tools/runtime.mk $(SRC_LIB)/boot/sega.s
	@$(MKDIR) -p $(dir $@)
	python3 tools/cart_boot.py $(SRC_LIB)/boot/sega.s $(OUT_DIR)/sega.s
	$(CC) $(AFLAGS) -c $(OUT_DIR)/sega.s -o $@
