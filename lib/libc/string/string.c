#include <string.h>
#include <limits.h>
#include <ctype.h>

size_t strlen(const char *str) 
{
	register const char *s;
	for (s = str; *s; ++s);
	return(s - str);
}

char* strcat(char* destination, const char* source){
    char* ptr = destination + strlen(destination);
    while (*source != '\0') {
        *ptr++ = *source++;
    }
    *ptr = '\0';
    return destination;    
}

char* strncat(char* destination, const char* source, size_t num) {
    char* ptr = destination + strlen(destination);
 
    while (*source != '\0' && num--) {
        *ptr++ = *source++;
    }
 
    *ptr = '\0';
 
    return destination;
}

int strcmp(const char* s1, const char* s2){
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char * s1, const char * s2, size_t n){
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

char* strstr(const char* string, const char* substring) {
    char *a, *b;
    b = substring;
    
	if (*b == 0) {
		return string;
    }

    for ( ; *string != 0; string += 1) {
		if (*string != *b) {
		    continue;
		}
		a = string;
		while (1) {
		    if (*b == 0) {
				return string;
		    }
		    if (*a++ != *b++) {
				break;
		    }
		}
		b = substring;
    }

    return NULL;
}

char* strnstr(const char* s, const char* find, size_t slen) {
 	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

#define DICT_LEN 256

unsigned int is_delim(char c, const char *delim)
{
    while(*delim != '\0')
    {
        if(c == *delim)
            return 1;
        delim++;
    }
    return 0;
}

char* strtok(char* srcString, const char* delim){
    static char *backup_string; // start of the next search
    if(!srcString)
    {
        srcString = backup_string;
    }
    if(!srcString)
    {
        // user is bad user
        return NULL;
    }
    // handle beginning of the string containing delims
    while(1)
    {
        if(is_delim(*srcString, delim))
        {
            srcString++;
            continue;
        }
        if(*srcString == '\0')
        {
            // we've reached the end of the string
            return NULL; 
        }
        break;
    }
    char *ret = srcString;
    while(1)
    {
        if(*srcString == '\0')
        {
            /*end of the input string and
            next exec will return NULL*/
            backup_string = srcString;
            return ret;
        }
        if(is_delim(*srcString, delim))
        {
            *srcString = '\0';
            backup_string = srcString + 1;
            return ret;
        }
        srcString++;
    }
}

long strtol(const char *nptr, char **endptr, int base) {
	const char *s;
	long acc, cutoff;
	int c;
	int neg, any, cutlim;

	s = nptr;
	
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));
	
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}

	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	
	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = neg ? LONG_MIN : LONG_MAX;
	cutlim = cutoff % base;
	cutoff /= base;
	if (neg) {
		if (cutlim > 0) {
			cutlim -= base;
			cutoff += 1;
		}
		cutlim = -cutlim;
	}
	for (acc = 0, any = 0;; c = (unsigned char) *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0)
			continue;
		if (neg) {
			if (acc < cutoff || (acc == cutoff && c > cutlim)) {
				any = -1;
				acc = LONG_MIN;
				// errno = ERANGE;
			} else {
				any = 1;
				acc *= base;
				acc -= c;
			}
		} else {
			if (acc > cutoff || (acc == cutoff && c > cutlim)) {
				any = -1;
				acc = LONG_MAX;
				// errno = ERANGE;
			} else {
				any = 1;
				acc *= base;
				acc += c;
			}
		}
	}
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}

char *strdup(const char *str){
#ifndef __LIBK
    char* buff = malloc(strlen(str) + 1);
    strcpy(buff, str);
	return buff;
 #else
	return NULL;
#endif
}

char *strchr(const char *s, int c){
    while (*s != (char) c) {
        if (!*s++) {
            return NULL;
        }
    }
    return (char *)s;
}

char *strchrnul(const char *s, int c) {
    while (*s) {
        if (c == *s)
            break;
        s++;
    }
    return (char *)(s);
}


