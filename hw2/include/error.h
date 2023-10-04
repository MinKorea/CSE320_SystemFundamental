extern int errors;
extern int warnings;

void fatal(const char *fmt, ...);
void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void debug(const char *fmt, ...);