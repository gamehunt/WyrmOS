#include "boot/limine.h"
#include "dev/log.h"
#include "fs/fs.h"
#include <dev/terminal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

typedef struct {
    void*   addr;
    size_t  w, h;
    size_t  pitch;
    uint8_t bpp;
} fb_info;

static fb_info  __framebuffer = {0};
static uint32_t cx = 0, cy = 0;

#define PSF_FONT_MAGIC 0x864ab572

typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;

extern char _binary____kernel_font_psf_start;
extern char _binary____kernel_font_psf_end;

static uint16_t* __unicode = NULL;

PSF_font* __font = (PSF_font*) &_binary____kernel_font_psf_start;

static int __load_font() {
    uint16_t glyph = 0;

    if (__font->magic != PSF_FONT_MAGIC) {
        return 0;
    }

    if (__font->flags) {
        return 1; 
    }

    /* get the offset of the table */
    char *s = (char *)(
    (unsigned char*)&_binary____kernel_font_psf_start +
      __font->headersize +
      __font->numglyph * __font->bytesperglyph
    );
    /* allocate memory for translation table */
    __unicode = calloc(0xFFFF, 2);
    while(s > _binary____kernel_font_psf_end) {
        uint16_t uc = (uint16_t)((unsigned char *)s[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        __unicode[uc] = glyph;
        s++;
    }
    return 1;
}

static void  __pixel(fb_info* info, uint32_t x, uint32_t y, uint32_t color) {
    *(uint32_t*)(info->addr + y * info->pitch + x * info->bpp / 8) = color;
}

static void __char(fb_info* info, uint32_t ox, uint32_t oy, uint16_t c) {
    if (__unicode && __unicode[c]) {
        c = __unicode[c];
    }

    uint8_t* glyph = (((uint8_t*) __font) + __font->headersize + c * __font->bytesperglyph);

    for (uint32_t y = 0; y < __font->height; y++) {
        for (uint32_t x = 0; x < __font->width; x++) {
            uint32_t glyph_row = glyph[y];

            if (glyph_row & (1 << (__font->width - x))) {
                __pixel(info, ox + x, oy + y, 0x00FFFFFF);
            } else {
                __pixel(info, ox + x, oy + y, 0x00000000);
            }
        }
    }
}

static void __scroll(fb_info* info, uint32_t pixels) {
    memmove(info->addr, info->addr + info->pitch * pixels, info->pitch * (info->h - pixels - 1));
    memset(info->addr + info->pitch * (info->h - pixels), 0, info->pitch * pixels);
}

static void __shift_down(fb_info* info) {
    cy++;
    if(cy * __font->height >= info->h) {
        __scroll(info, __font->height);
        cy = (info->h - __font->height) / __font->height;
    }
}

static void __shift_right(fb_info* info) {
    cx++;
    if(cx * __font->width > info->w) {
        cx = 0; 
        __shift_down(info);
    }
}

static void __putchar(fb_info* info, char c) {
        switch(c) {
            case '\n':
                __shift_down(info);
                break;
            case '\r':
                cx = 0;
                break;
            case '\t':
                __putchar(info, ' ');
                __putchar(info, ' ');
                __putchar(info, ' ');
                __putchar(info, ' ');
                break;
            default:
                __char(info, cx * __font->width, cy * __font->height, c);
                __shift_right(info);
        }
}

static size_t __k_write_terminal(fs_node* dev, size_t offset, size_t size, uint8_t* buffer) {
    for(size_t i = 0; i < size; i++) {
        __putchar(dev->meta, buffer[i]);
    }
    return size;
}

static void __terminal_log_listener(char c) {
    __putchar(&__framebuffer, c);
}

void k_dev_terminal_init() {
    if(!framebuffer_request.response || !framebuffer_request.response->framebuffer_count) {
        k_warn("No framebuffer provided by bootloader, will wait for drivers for explicit initialization");
        return;
    }

    if(!__load_font()){
        k_error("Failed to load kernel font.");
        return;
    }

    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    __framebuffer.addr  = fb->address;
    __framebuffer.w     = fb->width;
    __framebuffer.h     = fb->height;
    __framebuffer.pitch = fb->pitch;
    __framebuffer.bpp   = fb->bpp;
    
    fs_node* term_device = k_fs_alloc_fsnode("terminal");
    term_device->meta = &__framebuffer;
    term_device->ops.write = __k_write_terminal;
    k_fs_mount_node("/dev/terminal", term_device);

    k_dev_log_subscribe(__terminal_log_listener);
}
