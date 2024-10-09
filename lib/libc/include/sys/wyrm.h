#ifndef __SYS_WYRM_H
#define __SYS_WYRM_H 1

#ifdef __cplusplus
#define CHEADER_START extern "C" {
#define CHEADER_END }
#else
#define CHEADER_START
#define CHEADER_END
#endif

#define UNIMPL fprintf(stderr, "%s:%d %s(): unimplemented", __FILE__, __LINE__, __FUNCTION__); abort();

CHEADER_START
CHEADER_END

#endif
