;CD Boot Loader
;_______________________________________________________________________________

	org 0


%define TOTAL_SECTORS		(volume_end-$$)/2048

%define PATH_TABLE_SIZE (path_table_le_end - path_table_le_start + 3)

%define TIMESTRING db '2005010100000000',0
;		      'YYYYMMDDHHMMSS00',UTC offset

%define NULLTIMESTRING db '0000000000000000',0

%define TIMESTAMP db 105,1,1,0,0,0,0			;Midnight on 1/1/2005
							; Number of years since 1900
							; Month (1 = January)
							; Day of month (1 to 31)
							; Hour (0 to 23)
							; Minute (0 to 59)
							; Second (0 to 59)
							; UTC offset - signed 15 minute increments

%define BIG_ENDIAN(x) (((x) & 0x000000FF) << 24) + (((x) & 0x0000FF00) << 8) + (((x) & 0x00FF0000) >> 8) + (((x) & 0xFF000000) >> 24) 

%define SECTOR(x) ((x - $$) / 2048)


;ROOTDIR_ENTRY

%macro ROOTDIR_ENTRY 0
%%start:
	db %%end-%%start				;Size of entry
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(root_dir_start)			;First sector number of directory (little endian) **
	dd BIG_ENDIAN(SECTOR(root_dir_start))		;First sector number of directory (big endian) **
	dd 2048						;Number of directory entries (little endian)
	dd BIG_ENDIAN(2048)				;Number of directory entries (big endian)
	TIMESTAMP
	db 2						;Flags,
							; Bit 0:  0=normal file, 1=hidden
							; Bit 1:  0=file, 1=directory
							; Bit 2:  0=, 1=
							; Bit 3:  0=, 1=
							; Bit 4:  0=, 1=
							; Bit 5:  0=, 1=
							; Bit 6:  0=, 1=
							; Bit 7:  0=not final record for file, 1=final record for file
	db 0						;File unit size (for interleaved file)
	db 0						;Interleave gap size (for interleaved file)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	db 1						;Identifier length
	db 0						;The identifier/name
	align 2,db 0					;Padding byte if identifier length is even
							;Field for system use (unused)
%%end
%endmacro

;NEWDIR_ENTRY parent_directory_address

%macro NEWDIR_ENTRY 1
%%start1:
	db %%end1-%%start1				;Size of entry
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(%%start1)				;First sector number of directory (little endian) **
	dd BIG_ENDIAN(SECTOR(%%start1))			;First sector number of directory (big endian) **
	dd 2048						;Number of directory entries (little endian)
	dd BIG_ENDIAN(2048)				;Number of directory entries (big endian)
	TIMESTAMP
	db 2						;Flags,
							; Bit 0:  0=normal file, 1=hidden
							; Bit 1:  0=file, 1=directory
							; Bit 2:  0=, 1=
							; Bit 3:  0=, 1=
							; Bit 4:  0=, 1=
							; Bit 5:  0=, 1=
							; Bit 6:  0=, 1=
							; Bit 7:  0=not final record for file, 1=final record for file
	db 0						;File unit size (for interleaved file)
	db 0						;Interleave gap size (for interleaved file)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	db 1						;Identifier length
	db 0						;The identifier/name
	align 2,db 0					;Padding byte if identifier length is even
							;Field for system use (unused)
%%end1:
%%start2:
	db %%end2-%%start2				;Size of entry
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(%1)					;First sector number of directory (little endian) **
	dd BIG_ENDIAN(SECTOR(%1))			;First sector number of directory (big endian) **
	dd 2048						;Number of directory entries (little endian)
	dd BIG_ENDIAN(2048)				;Number of directory entries (big endian)
	TIMESTAMP
	db 2						;Flags,
							; Bit 0:  0=normal file, 1=hidden
							; Bit 1:  0=file, 1=directory
							; Bit 2:  0=, 1=
							; Bit 3:  0=, 1=
							; Bit 4:  0=, 1=
							; Bit 5:  0=, 1=
							; Bit 6:  0=, 1=
							; Bit 7:  0=not final record for file, 1=final record for file
	db 0						;File unit size (for interleaved file)
	db 0						;Interleave gap size (for interleaved file)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	db 1						;Identifier length
	db 0						;The identifier/name
	align 2,db 0					;Padding byte if identifier length is even
							;Field for system use (unused)
%%end2:
%endmacro


;FILEDIR_ENTRY name, start_address, end_address

%macro FILEDIR_ENTRY 3
%%start:
	db %%end-%%start				;Size of entry
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(%2)					;First sector number of directory (little endian) **
	dd BIG_ENDIAN(SECTOR(%2))			;First sector number of directory (big endian) **
	dd (%3-%2)					;Length of file (little endian) **
	dd BIG_ENDIAN(%3-%2)				;Length of file (big endian) **
	TIMESTAMP
	db 0						;Flags,
							; Bit 0:  0=normal file, 1=hidden
							; Bit 1:  0=file, 1=directory
							; Bit 2:  0=, 1=
							; Bit 3:  0=, 1=
							; Bit 4:  0=, 1=
							; Bit 5:  0=, 1=
							; Bit 6:  0=, 1=
							; Bit 7:  0=not final record for file, 1=final record for file
	db 0						;File unit size (for interleaved file)
	db 0						;Interleave gap size (for interleaved file)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	db %%name_end - %%name_start			;Identifier length
%%name_start:
	db %1						;The identifier/name
%%name_end
	align 2,db 0					;Padding byte if identifier length is even
							;Field for system use (unused)
%%end
%endmacro


;SUBDIR_ENTRY name, start_address, end_address

%macro SUBDIR_ENTRY 3
%%start:
	db %%end-%%start				;Size of entry
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(%2)					;First sector number of directory (little endian) **
	dd BIG_ENDIAN(SECTOR(%2))			;First sector number of directory (big endian) **
	dd (%3-%2)					;Length of file (little endian) **
	dd BIG_ENDIAN(%3-%2)				;Length of file (big endian) **
	TIMESTAMP
	db 2						;Flags,
							; Bit 0:  0=normal file, 1=hidden
							; Bit 1:  0=file, 1=directory
							; Bit 2:  0=, 1=
							; Bit 3:  0=, 1=
							; Bit 4:  0=, 1=
							; Bit 5:  0=, 1=
							; Bit 6:  0=, 1=
							; Bit 7:  0=not final record for file, 1=final record for file
	db 0						;File unit size (for interleaved file)
	db 0						;Interleave gap size (for interleaved file)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	db %%name_end - %%name_start			;Identifier length
%%name_start:
	db %1						;The identifier/name
%%name_end
	align 2,db 0					;Padding byte if identifier length is even
							;Field for system use (unused)
%%end
%endmacro


;PATH_LE address_of_sub_directory, parent, name

%macro PATH_LE 3
	db %%name2-%%name1				;Name length (1 for root directory)
	db 0						;Number of sectors in extended attribute record
	dd SECTOR(%1)					;First sector in the directory **
	dw %2						;Number of parent directory (1 for root directory)
%%name1:	
	db %3						;ASCII name string (0 for root directory)
%%name2:
	align 2,db 0					;Padding (only if name string length is odd)
%endmacro


;PATH_BE address_of_sub_directory, parent, name

%macro PATH_BE 3
	db %%name2-%%name1				;Name length (1 for root directory)
	db 0						;Number of sectors in extended attribute record
	dd BIG_ENDIAN(SECTOR(%1))			;First sector in the directory **
	dw BIG_ENDIAN(%2)				;Number of parent directory (1 for root directory)
%%name1:	
	db %3						;ASCII name string (0 for root directory)
%%name2:
	align 2,db 0					;Padding (only if name string length is odd)
%endmacro


;_______________________________________________________________________________


;Skip the first 16 sectors

	times ($$-$+16*2048) db 0x00

;Sector 16 - Primary Volume Descriptor

	db 1
	db 'CD001'					;ISO9660 identifier
	db 1						;Version descriptor
	db 0
	db '                                '		;System identifier
	db 'BCOS INSTALLATION CD            '		;Volume identifier
	times 8 db 0x00					;Zeroes
	dd TOTAL_SECTORS				;Total sectors (little endian)
	dd BIG_ENDIAN(TOTAL_SECTORS)			;Total sectors (big endian)
	times 32 db 0x00				;Zeroes
	dw 0x0001					;Volume set size (little endian)
	dw 0x0100					;Volume set size (big endian)
	dw 0x0001					;Volume sequence number (little endian)
	dw 0x0100					;Volume sequence number (big endian)
	dw 0x0800					;Sector size (little endian)
	dw 0x0008					;Sector size (big endian)
	dd PATH_TABLE_SIZE				;Path table length in bytes (little endian) **
	dd BIG_ENDIAN(PATH_TABLE_SIZE)			;Path table length in bytes (big endian) **
	dd SECTOR(path_table_le_start)			;Sector number for first little endian path table (little endian) **
	dd 0x00000000					;Sector number for second little endian path table (little endian) **
	dd BIG_ENDIAN(SECTOR(path_table_be_start))	;Sector number for first big endian path table (big endian) **
	dd 0x00000000					;Sector number for second big endian path table (big endian) **
	ROOTDIR_ENTRY					;Root Directory Entry
	times 128 db 0x20				;Volume Set Identifier

	db 'PREPARED BY BRENDAN TROTTER VIA.'		;Publisher Identifier
	db ' HAND CODED NASM SOURCE CODE FOR'
	db ' THE USE OF THE BCOS OPERATING S'
	db 'YSTEM.                          '

	db 'PREPARED BY BRENDAN TROTTER VIA.'		;Data Preparer Identifier
	db ' HAND CODED NASM SOURCE CODE FOR'
	db ' THE USE OF THE BCOS OPERATING S'
	db 'YSTEM.                          '

	db 'PREPARED BY BRENDAN TROTTER VIA.'		;Application Identifier
	db ' HAND CODED NASM SOURCE CODE FOR'
	db ' THE USE OF THE BCOS OPERATING S'
	db 'YSTEM.                          '

	times 37 db 0x20				;Copyright File Identifier
	times 37 db 0x20				;Abstract File Identifier
	times 37 db 0x20				;Bibliographical File Identifier
	TIMESTRING					;Date and time of volume creation
	TIMESTRING					;Date and time of most recent modification
	NULLTIMESTRING					;Date and time of volume expiry
	TIMESTRING					;Date and time when volume is effective
	db 1						;File structure revision
	db 0						;Reserved for future standardization
	times 512 db 0x20				;Reserved for my use
	align 2048,db 0


;Sector 17 - Boot Record Volume Descriptor

	db 0x00						;Boot record indicator (must be 0)
	db 'CD001'					;ISO9660 identifier
	db 1						;Version descriptor
	db 'EL TORITO SPECIFICATION'			;Boot system identifier
	db 0,0,0,0,0,0,0,0,0				;Padding (rest of boot system identifier)
	times 32 db 0x00				;Unused (must be 0)
	dd SECTOR(boot_catalog_start)			;** Absolute pointer to first sector of Boot Catalog
	align 2048,db 0

;Sector 18 - Volume Descriptor Set Terminator

	db 0xFF						;Boot record indicator (must be 0)
	db 'CD001'					;ISO9660 identifier
	db 1						;Version descriptor
	align 2048,db 0


;Sector 19 - Boot Catalog

boot_catalog_start:

	;Validation Entry

	db 1						;Header ID
	db 0						;Platform (0 = 80x86)
	db 0,0						;Reserved (must be 0)
	db 'BCOS                    '			;ID string
	dw 0x1284					;Checksum
	db 0x55						;Key byte 1
	db 0xAA						;Key byte 2

	;Initial/Default Entry

	db 0x88						;Bootable indicator
	db 2						;Boot media type (4 = hard drive emulation)
	dw 0x07c0					;Load segment
	db 0						;System type (same as partition table byte 5)
	db 0						;Unused (must be 0)
	dw 1						;512 byte sector count (4 = 2048 bytes)
	dw SECTOR(start_floppy_144)			;Sector for start of virtual disk
	dd 0,0,0,0,0					;Unused (must be 0)

	align 2048,db 0


;_______________________________________________________________________________


;Sector 20 - Root Directory

root_dir_start:
	NEWDIR_ENTRY root_dir_start
	FILEDIR_ENTRY "AUTORUN.INF;1", start_autorun_inf, end_autorun_inf
	SUBDIR_ENTRY "DIST;1", dist_dir_start, dist_dir_end
	SUBDIR_ENTRY "DOS;1", dos_dir_start, dos_dir_end
	FILEDIR_ENTRY "FLOPPY.144;1", start_floppy_144, end_floppy_144
	FILEDIR_ENTRY "INSTALL.IMG;1", start_install_img, end_install_img
	FILEDIR_ENTRY "README.HTM;1", start_readme_htm, end_readme_htm
	FILEDIR_ENTRY "README.TXT;1", start_readme_txt, end_readme_txt
	SUBDIR_ENTRY "WINDOWS;1", win_dir_start, win_dir_end
	align 2048,db 0


;Sub directory - DIST

dist_dir_start:
	NEWDIR_ENTRY root_dir_start
dist_dir_end:
	align 2048,db 0

;Sub directory - DOS

dos_dir_start:
	NEWDIR_ENTRY root_dir_start
	FILEDIR_ENTRY "BOOT.COM;1", start_boot_com, end_boot_com
	FILEDIR_ENTRY "MAKEBOOT.COM;1", start_makeflop_com, end_makeflop_com
dos_dir_end:
	align 2048,db 0

;Sub directory - WINDOWS

win_dir_start:
	NEWDIR_ENTRY root_dir_start
	FILEDIR_ENTRY "COPYING;1", start_copying, end_copying
	FILEDIR_ENTRY "DISKIO.DLL;1", start_rawrite_dll, end_rawrite_dll
	FILEDIR_ENTRY "RAWWRITE.EXE;1", start_rawrite_exe, end_rawrite_exe
win_dir_end:
	align 2048,db 0


;_______________________________________________________________________________


;Little Endian Path Table

path_table_le_start:
	PATH_LE root_dir_start, 1, 0
	PATH_LE dist_dir_start, 1, "DIST"
	PATH_LE dos_dir_start, 1, "DOS"
	PATH_LE win_dir_start, 1, "WINDOWS"
path_table_le_end:
	align 2048,db 0


;Big Endian Path Table

path_table_be_start:
	PATH_BE root_dir_start, 1, 0
	PATH_BE dist_dir_start, 1, "DIST"
	PATH_BE dos_dir_start, 1, "DOS"
	PATH_BE win_dir_start, 1, "WINDOWS"
	align 2048,db 0

;_______________________________________________________________________________


;Files for DIST

;Files for DOS

;DOSBOOT.COM

start_boot_com:
	incbin "../bin/dos/boot.com"
end_boot_com:
	align 2048,db 0

;MAKEFLOP.COM

start_makeflop_com:
	incbin "../bin/dos/makeflop.com"
end_makeflop_com:
	align 2048,db 0


;Files for WINDOWS

start_copying:
	incbin "COPYING"
end_copying:
	align 2048,db 0

start_rawrite_dll:
	incbin "diskio.dll"
end_rawrite_dll:
	align 2048,db 0

start_rawrite_exe:
	incbin "rawwritewin.exe"
end_rawrite_exe:
	align 2048,db 0


;Files for root directory

;AUTORUN.INF

start_autorun_inf:
	incbin "autorun.inf"
end_autorun_inf:
	align 2048,db 0

;FLOPPY.144

start_floppy_144:
	incbin "../bin/floppy.144"
	times (1474560+start_floppy_144-$) db 0x00
end_floppy_144:
	align 2048,db 0


;INSTALL.IMG

start_install_img:
	incbin "../bin/install.img"
end_install_img:
	align 2048,db 0

	
;README.HTM

start_readme_htm:
	incbin "readme.htm"
end_readme_htm:
	align 2048,db 0

	
;README.TXT

start_readme_txt:
	incbin "readme.txt"
end_readme_txt:
	align 2048,db 0
	
volume_end:


