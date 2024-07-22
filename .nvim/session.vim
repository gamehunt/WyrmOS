let SessionLoad = 1
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd /hdd/Projects/WyrmOS
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
let s:shortmess_save = &shortmess
if &shortmess =~ 'A'
  set shortmess=aoOA
else
  set shortmess=aoO
endif
badd +32 /hdd/Projects/WyrmOS/kernel/kernel.c
badd +14 /hdd/Projects/WyrmOS/kernel/include/asm.h
badd +33 /hdd/Projects/WyrmOS/kernel/asm.c
badd +540 /hdd/Projects/WyrmOS/kernel/include/boot/limine.h
badd +6 /hdd/Projects/WyrmOS/kernel/include/globals.h
badd +3 /hdd/Projects/WyrmOS/kernel/globals.c
badd +7 /hdd/Projects/WyrmOS/kernel/meson.build
badd +17 /hdd/Projects/WyrmOS/lib/libc/include/stdio.h
badd +454 /hdd/Projects/WyrmOS/lib/libc/stdio/printf.c
badd +8 /hdd/Projects/WyrmOS/lib/libc/meson.build
badd +28 /hdd/Projects/WyrmOS/meson.build
badd +1 /hdd/Projects/WyrmOS/lib/libc/include/ctype.h
badd +29 /hdd/Projects/WyrmOS/lib/libc/ctype/ctype.c
badd +4 /hdd/Projects/WyrmOS/lib/libc/include/stdlib.h
badd +29 /hdd/Projects/WyrmOS/lib/libc/stdlib/stdlib.c
badd +1 /hdd/Projects/WyrmOS/lib/libc/include/string.h
badd +1 /hdd/Projects/WyrmOS/kernel/include/debug.h
badd +14 /hdd/Projects/WyrmOS/kernel/include/dev/serial.h
badd +25 /hdd/Projects/WyrmOS/kernel/dev/serial.c
badd +1 /hdd/Projects/WyrmOS/README.md
badd +2 /hdd/Projects/WyrmOS/kernel/include/cpu/interrupt.h
badd +1 /hdd/Projects/WyrmOS/kernel/cpu/interrupt.c
badd +20 /hdd/Projects/WyrmOS/kernel/include/mem/pmm.h
badd +4 /hdd/Projects/WyrmOS/kernel/include/mem/paging.h
badd +19 /hdd/Projects/WyrmOS/kernel/mem/pmm.c
badd +1 /hdd/Projects/WyrmOS/kernel/mem/paging.c
badd +9 /hdd/Projects/WyrmOS/kernel/include/mem/mem.h
badd +11 /hdd/Projects/WyrmOS/kernel/mem/mem.c
badd +47 /hdd/Projects/WyrmOS/kernel/link.ld
argglobal
%argdel
edit /hdd/Projects/WyrmOS/kernel/mem/pmm.c
let s:save_splitbelow = &splitbelow
let s:save_splitright = &splitright
set splitbelow splitright
wincmd _ | wincmd |
split
1wincmd k
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
wincmd w
let &splitbelow = s:save_splitbelow
let &splitright = s:save_splitright
wincmd t
let s:save_winminheight = &winminheight
let s:save_winminwidth = &winminwidth
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe '1resize ' . ((&lines * 29 + 22) / 45)
exe 'vert 1resize ' . ((&columns * 30 + 105) / 210)
exe '2resize ' . ((&lines * 29 + 22) / 45)
exe 'vert 2resize ' . ((&columns * 179 + 105) / 210)
exe '3resize ' . ((&lines * 12 + 22) / 45)
argglobal
enew
file NvimTree_1
balt /hdd/Projects/WyrmOS/kernel/kernel.c
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal nofen
wincmd w
argglobal
balt /hdd/Projects/WyrmOS/kernel/include/boot/limine.h
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let &fdl = &fdl
let s:l = 15 - ((14 * winheight(0) + 14) / 29)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 15
normal! 0
wincmd w
argglobal
if bufexists(fnamemodify("term:///hdd/Projects/WyrmOS//14831:/usr/bin/zsh;\#toggleterm\#1", ":p")) | buffer term:///hdd/Projects/WyrmOS//14831:/usr/bin/zsh;\#toggleterm\#1 | else | edit term:///hdd/Projects/WyrmOS//14831:/usr/bin/zsh;\#toggleterm\#1 | endif
if &buftype ==# 'terminal'
  silent file term:///hdd/Projects/WyrmOS//14831:/usr/bin/zsh;\#toggleterm\#1
endif
balt /hdd/Projects/WyrmOS/lib/libc/stdio/printf.c
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
let s:l = 1643 - ((11 * winheight(0) + 6) / 12)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 1643
normal! 03|
wincmd w
2wincmd w
exe '1resize ' . ((&lines * 29 + 22) / 45)
exe 'vert 1resize ' . ((&columns * 30 + 105) / 210)
exe '2resize ' . ((&lines * 29 + 22) / 45)
exe 'vert 2resize ' . ((&columns * 179 + 105) / 210)
exe '3resize ' . ((&lines * 12 + 22) / 45)
tabnext 1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0 && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20
let &shortmess = s:shortmess_save
let &winminheight = s:save_winminheight
let &winminwidth = s:save_winminwidth
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
set hlsearch
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
