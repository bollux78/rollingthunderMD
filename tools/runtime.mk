# Redirect only SGDK's warm-reset entry. Cold boot keeps the SDK startup path.
LIBGCC += -Wl,--wrap=_reset_entry
