void mcp23018_init(void);

// Note: No proper state checking. Will fail hard if called inappropriately.
void mcp23018_begin(void);

// Idles until LH scan is done.
uint8_t mcp23018_poll(void);