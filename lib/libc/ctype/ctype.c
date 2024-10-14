#include <ctype.h>

int isspace(int c) {
    return (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ' ? 1 : 0);
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
} 

int isalpha(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); 
}

int isblank(int c) {
    return isspace(c);
}

int iscntrl(int c) {
    return c == 0x7F || (c >= 0 && c <= 0x1F); 
}

int isdigit(int c) {
    return (c >= '0' && c <= '9'); 
}

int isgraph(int c) {
    return c >= 0x21 && c <= 0x7E;
}

int islower(int c) {
    return (c >= 'a') && (c <= 'z');
}

int isupper(int c) {
    return (c >= 'A') && (c <= 'Z');
} 

int isprint(int c) {
    return c >= 0x20 && c <= 0x7E;
}

int ispunct(int c) {
    return c != ' ' && !isalnum(c) && isprint(c);
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int toupper(int c) {
    if(islower(c)) {
        return c - 'a' + 'A';
    } else {
        return c;
    }
} 

int tolower(int c) {
    if(isupper(c)) {
        return c - 'A' + 'a';
    } else {
        return c;
    }
}  
