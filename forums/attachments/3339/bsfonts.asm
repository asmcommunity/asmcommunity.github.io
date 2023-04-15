;From: "Beth" <BethStone21@hotmail.NOSPICEDHAM.com>
                        ; Balloon.asm
                        ; (C)2K1 Beth (but donated to the public domain :)
                        ;
                        ; Simple .com file that demonstrates changing the
                        ; video font for text mode 3 using the "official"
                        ; BIOS interrupts for doing it...
                        ;
                        ; Note: mode 3 is a bit polymorphic in that the size
                        ; of a character cell differs according to the video
                        ; card...but, for modern VGAs and above, these will
                        ; almost certainly have a 9x16 size character cell by
                        ; default (but the older cards like CGA/EGA and such had
                        ; a lower resolution and an 8x8 character cell)...that
                        ; is, mode 3 is defined by the amount of character
                        ; cells being 80x25 and the card alters the size of the
                        ; character cell to make the resolution and this
                        ; requirement correspond...
                        ;
                        ; As this is the case, I force the character cell to be
                        ; 8x8 in this little program to make life easier in that
                        ; I don't have to supply fonts for 8x8, 8x14, 8x16
                        ; potential sizes...this, on a modern VGA and above,
                        ; will force the number of rows on screen to 50 and
                        ; produce shorter characters than usual...
                        ;
                        ; Finally, it's worth noting that the VGA(+) resolution
                        ; of mode 3 is actually 720 pixels across rather than
                        ; what you might assume of 640 (because we've 80
                        ; columns each at 8 pixels wide and 80x8=640, which is
                        ; not 720 :)...thus, these cards automatically insert
                        ; a vertical gap of one pixel between the character
                        ; cells (if you look at ASCII characters 176, 177,178
                        ; and 219, you can see this in action...because
                        ; although 219 is defined as "all pixels on" (a solid
                        ; block) when you make a row of them, you'll see a gap
                        ; between them...this you can see on my little balloon
                        ; graphic also...
                        ;
                        ; Also, if you're going to run this from a DOS box in
                        ; Windows, then you'll need to go into fullscreen mode
                        ; because Windows _emulates_ text modes in a window
                        ; and, thus, my little balloon won't show up (what's
                        ; interesting, though, is that if you switch between
                        ; fullscreen and a window with ALT + ENTER then you
                        ; can see the difference...in fullscreen, you'll see
                        ; the balloon but in a window, you'll just see weird
                        ; extended ASCII characters)...
                        ;
                        ; Anyway, enough of these overly long comments (there's
                        ; more comment here than actual program...hehehe ;)...
                        ; let's get to some actual code...
                        ;
;                        .model tiny
;                        .code

%define offset
    section .text
                        org   100h

                        ; Set the video mode to the standard text mode #3
                        ;
                 Start: mov   ax, 0003h
                        int   10h

                        ; Load in the standard 8x8 ROM font...which also forces
                        ; the video to use 8x8 character cells...
                        ;
                        mov   ax, 1112h
;                        int   10h

                        ; Ok, let's load in my new font characters...
                        ;
                        mov   ax, 1110h
                        mov   bx, 0800h
                        mov   cx, 0007h
                        mov   dx, 0080h
                        mov   bp, offset NewFont
                        int   10h

                        ; And let's print some characters to demonstrate my new
                        ; font in action...
                        mov   ax, 0900h
                        mov   dx, offset Balloon
                        int   21h

                        ; Goodbye program, hello DOS prompt!!!
                        ; (i.e. call terminate program interrupt :)
                        ;
                        mov   ah, 4Ch
                        int   21h

                Balloon db    0Ah, 0Dh, "Look! It's a balloon!", 0Ah, 0Dh
                        db    "But, wait, we're in a text mode but that looks "
                        db    "a lot like a graphic...", 0Ah, 0Dh,
                        db    "how can this be?", 0Ah, 0Dh
                        db    "Cool, huh? :)", 0Ah, 0Dh, 0Ah, 0Dh
                        db    128, 129, 130, 0Ah, 0Dh
                        db    131, 132, 133, 0Ah, 0Dh
                        db    " ", 134, " ", 0Ah, 0Dh, 0Ah, 0Dh, "$"

                        ; Here's the data for the new characters I'm
                        ; defining...the format is a simple monochrome
                        ; bitmap, where each bit in a byte represents a
                        ; pixel (0 = off, 1 = on :) and there's 8 bytes
                        ; to make up an 8x8 character...
                        ;
                        ; As I can't be bothered to calculate some interesting
                        ; graphics of my own here, then I've loaned the already
                        ; calculated values from the C64 user's guide for
                        ; creating a balloon sprite (which, coincidentally,
                        ; uses a similar format to the format I need here, so
                        ; it saves time just to copy those values over rather
                        ; than work out some of my own :)...just thought I'd
                        ; explain why I've got a balloon with the Commodore
                        ; logo on it as my graphic...the answer is simply that
                        ; I was too lazy to sit with some graph paper and work
                        ; out some values of my own...trust me, I've done it
                        ; before on other occasions and it's a really long and
                        ; tedious process :)...
                        ;
                NewFont db    0, 1, 3, 3, 7, 7, 7, 3
                        db    127, 255, 255, 231, 217, 223, 217, 231
                        db    0, 192, 224, 224, 240, 240, 240, 224

                        db    3, 3, 2, 1, 1, 0, 0, 0
                        db    255, 255, 255, 127, 62, 156, 156, 73
                        db    224, 224, 160, 64, 64, 128, 128, 0

                        db    73, 62, 62, 62, 28, 0, 0, 0

;                        end   Start
