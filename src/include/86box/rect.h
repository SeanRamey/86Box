#ifndef RECT_H
#define RECT_H

#ifdef __cplusplus
extern "C" {
#endif


// Windows WINAPI RECT compatible struct for cross platform code.
typedef struct Rect {
	int left;
	int top;
	int right;
	int bottom;
} Rect;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* RECT_H */
