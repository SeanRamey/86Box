#include <string.h>

#include <86box/utility.h>

/*
* UTF8 aware strncpy() similar function.
* Does a reverse search of src (starts at the null terminator)
* and works backward to find the last full UTF8 character which
* can fit into the destination buffer.
* It always ensures the destination is null terminated.
*/
char* utf8cpy(char* dst, const char* src, size_t sizeDest)
{
    if( sizeDest ){
        size_t sizeSrc = strlen(src); // number of bytes not including null
        while( sizeSrc >= sizeDest ){

            const char* lastByte = src + sizeSrc; // Initially, pointing to the null terminator.
            while( lastByte-- > src )
                if((*lastByte & 0xC0) != 0x80) // Found the initial byte of the (potentially) multi-byte character (or found null).
                    break;

            sizeSrc = lastByte - src;
        }
        memcpy(dst, src, sizeSrc);
        dst[sizeSrc] = '\0';
    }
    return dst;
}
