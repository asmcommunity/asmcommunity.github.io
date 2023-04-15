;*****************************
;***** The NASMX Project *****
;*****************************

%ifndef __NASMX_SYSCALL_INC__
%define __NASMX_SYSCALL_INC__

%if __NASMX_ABI_BITS__ == 64

; ---------------------------------
; SysCall Macro
; ---------------------------------

%imacro syscall 1-7
  %if %0 > 1
    %if %0 > 2
      %if %0 > 3
        %if %0 > 4
          %if %0 > 5
            %if %0 > 6
              mov r9, %{7}
            %endif
            mov r8, %{6}
          %endif
          mov r10, %{5}
        %endif
        mov rdx, %{4}
      %endif
      mov rsi, %{3}
    %endif
    mov rdi, %{2}
  %endif
  mov rax, SYS_ %+ %{1}
  syscall  ; the opcode, not the macro
%endmacro

; ---------------------------------
; System Call Listing
; ---------------------------------

%define SYS_read				0
%define SYS_write				1
%define SYS_open				2
%define SYS_close				3
%define SYS_stat				4
%define SYS_fstat				5
%define SYS_lstat				6
%define SYS_poll				7
%define SYS_lseek				8
%define SYS_mmap				9
%define SYS_mprotect				10
%define SYS_munmap				11
%define SYS_brk				12
%define SYS_rt_sigaction			13
%define SYS_rt_sigprocmask			14
%define SYS_rt_sigreturn			15
%define SYS_ioctl				16
%define SYS_pread64				17
%define SYS_pwrite64				18
%define SYS_readv				19
%define SYS_writev				20
%define SYS_access				21
%define SYS_pipe				22
%define SYS_select				23
%define SYS_sched_yield			24
%define SYS_mremap				25
%define SYS_msync				26
%define SYS_mincore				27
%define SYS_madvise				28
%define SYS_shmget				29
%define SYS_shmat				30
%define SYS_shmctl				31
%define SYS_dup				32
%define SYS_dup2				33
%define SYS_pause				34
%define SYS_nanosleep				35
%define SYS_getitimer				36
%define SYS_alarm				37
%define SYS_setitimer				38
%define SYS_getpid				39
%define SYS_sendfile				40
%define SYS_socket				41
%define SYS_connect				42
%define SYS_accept				43
%define SYS_sendto				44
%define SYS_recvfrom				45
%define SYS_sendmsg				46
%define SYS_recvmsg				47
%define SYS_shutdown				48
%define SYS_bind				49
%define SYS_listen				50
%define SYS_getsockname			51
%define SYS_getpeername			52
%define SYS_socketpair				53
%define SYS_setsockopt				54
%define SYS_getsockopt				55
%define SYS_clone				56
%define SYS_fork				57
%define SYS_vfork				58
%define SYS_execve				59
%define SYS_exit				60
%define SYS_wait4				61
%define SYS_kill				62
%define SYS_uname				63
%define SYS_semget				64
%define SYS_semop				65
%define SYS_semctl				66
%define SYS_shmdt				67
%define SYS_msgget				68
%define SYS_msgsnd				69
%define SYS_msgrcv				70
%define SYS_msgctl				71
%define SYS_fcntl				72
%define SYS_flock				73
%define SYS_fsync				74
%define SYS_fdatasync				75
%define SYS_truncate				76
%define SYS_ftruncate				77
%define SYS_getdents				78
%define SYS_getcwd				79
%define SYS_chdir				80
%define SYS_fchdir				81
%define SYS_rename				82
%define SYS_mkdir				83
%define SYS_rmdir				84
%define SYS_creat				85
%define SYS_link				86
%define SYS_unlink				87
%define SYS_symlink				88
%define SYS_readlink				89
%define SYS_chmod				90
%define SYS_fchmod				91
%define SYS_chown				92
%define SYS_fchown				93
%define SYS_lchown				94
%define SYS_umask				95
%define SYS_gettimeofday			96
%define SYS_getrlimit				97
%define SYS_getrusage				98
%define SYS_sysinfo				99
%define SYS_times				100
%define SYS_ptrace				101
%define SYS_getuid				102
%define SYS_syslog				103
%define SYS_getgid				104
%define SYS_setuid				105
%define SYS_setgid				106
%define SYS_geteuid				107
%define SYS_getegid				108
%define SYS_setpgid				109
%define SYS_getppid				110
%define SYS_getpgrp				111
%define SYS_setsid				112
%define SYS_setreuid				113
%define SYS_setregid				114
%define SYS_getgroups				115
%define SYS_setgroups				116
%define SYS_setresuid				117
%define SYS_getresuid				118
%define SYS_setresgid				119
%define SYS_getresgid				120
%define SYS_getpgid				121
%define SYS_setfsuid				122
%define SYS_setfsgid				123
%define SYS_getsid				124
%define SYS_capget				125
%define SYS_capset				126
%define SYS_rt_sigpending			127
%define SYS_rt_sigtimedwait			128
%define SYS_rt_sigqueueinfo			129
%define SYS_rt_sigsuspend			130
%define SYS_sigaltstack			131
%define SYS_utime				132
%define SYS_mknod				133
%define SYS_uselib				134
%define SYS_personality			135
%define SYS_ustat				136
%define SYS_statfs				137
%define SYS_fstatfs				138
%define SYS_sysfs				139
%define SYS_getpriority			140
%define SYS_setpriority			141
%define SYS_sched_setparam			142
%define SYS_sched_getparam			143
%define SYS_sched_setscheduler			144
%define SYS_sched_getscheduler			145
%define SYS_sched_get_priority_max		146
%define SYS_sched_get_priority_min		147
%define SYS_sched_rr_get_interval		148
%define SYS_mlock				149
%define SYS_munlock				150
%define SYS_mlockall				151
%define SYS_munlockall				152
%define SYS_vhangup				153
%define SYS_modify_ldt				154
%define SYS_pivot_root				155
%define SYS__sysctl				156
%define SYS_prctl				157
%define SYS_arch_prctl				158
%define SYS_adjtimex				159
%define SYS_setrlimit				160
%define SYS_chroot				161
%define SYS_sync				162
%define SYS_acct				163
%define SYS_settimeofday			164
%define SYS_mount				165
%define SYS_umount2				166
%define SYS_swapon				167
%define SYS_swapoff				168
%define SYS_reboot				169
%define SYS_sethostname			170
%define SYS_setdomainname			171
%define SYS_iopl				172
%define SYS_ioperm				173
%define SYS_create_module			174
%define SYS_init_module			175
%define SYS_delete_module			176
%define SYS_get_kernel_syms			177
%define SYS_query_module			178
%define SYS_quotactl				179
%define SYS_nfsservctl				180
%define SYS_getpmsg				181
%define SYS_putpmsg				182
%define SYS_afs_syscall			183
%define SYS_tuxcall				184
%define SYS_security				185
%define SYS_gettid				186
%define SYS_readahead				187
%define SYS_setxattr				188
%define SYS_lsetxattr				189
%define SYS_fsetxattr				190
%define SYS_getxattr				191
%define SYS_lgetxattr				192
%define SYS_fgetxattr				193
%define SYS_listxattr				194
%define SYS_llistxattr				195
%define SYS_flistxattr				196
%define SYS_removexattr			197
%define SYS_lremovexattr			198
%define SYS_fremovexattr			199
%define SYS_tkill				200
%define SYS_time				201
%define SYS_futex				202
%define SYS_sched_setaffinity			203
%define SYS_sched_getaffinity			204
%define SYS_set_thread_area			205
%define SYS_io_setup				206
%define SYS_io_destroy				207
%define SYS_io_getevents			208
%define SYS_io_submit				209
%define SYS_io_cancel				210
%define SYS_get_thread_area			211
%define SYS_lookup_dcookie			212
%define SYS_epoll_create			213
%define SYS_epoll_ctl_old			214
%define SYS_epoll_wait_old			215
%define SYS_remap_file_pages			216
%define SYS_getdents64				217
%define SYS_set_tid_address			218
%define SYS_restart_syscall			219
%define SYS_semtimedop				220
%define SYS_fadvise64				221
%define SYS_timer_create			222
%define SYS_timer_settime			223
%define SYS_timer_gettime			224
%define SYS_timer_getoverrun			225
%define SYS_timer_delete			226
%define SYS_clock_settime			227
%define SYS_clock_gettime			228
%define SYS_clock_getres			229
%define SYS_clock_nanosleep			230
%define SYS_exit_group				231
%define SYS_epoll_wait				232
%define SYS_epoll_ctl				233
%define SYS_tgkill				234
%define SYS_utimes				235
%define SYS_vserver				236
%define SYS_mbind				237
%define SYS_set_mempolicy			238
%define SYS_get_mempolicy			239
%define SYS_mq_open				240
%define SYS_mq_unlink				241
%define SYS_mq_timedsend			242
%define SYS_mq_timedreceive			243
%define SYS_mq_notify				244
%define SYS_mq_getsetattr			245
%define SYS_kexec_load				246
%define SYS_waitid				247
%define SYS_add_key				248
%define SYS_request_key			249
%define SYS_keyctl				250
%define SYS_ioprio_set				251
%define SYS_ioprio_get				252
%define SYS_inotify_init			253
%define SYS_inotify_add_watch			254
%define SYS_inotify_rm_watch			255
%define SYS_migrate_pages			256
%define SYS_openat				257
%define SYS_mkdirat				258
%define SYS_mknodat				259
%define SYS_fchownat				260
%define SYS_futimesat				261
%define SYS_newfstatat				262
%define SYS_unlinkat				263
%define SYS_renameat				264
%define SYS_linkat				265
%define SYS_symlinkat				266
%define SYS_readlinkat				267
%define SYS_fchmodat				268
%define SYS_faccessat				269
%define SYS_pselect6				270
%define SYS_ppoll				271
%define SYS_unshare				272
%define SYS_set_robust_list			273
%define SYS_get_robust_list			274
%define SYS_splice				275
%define SYS_tee				276
%define SYS_sync_file_range			277
%define SYS_vmsplice				278
%define SYS_move_pages				279
%define SYS_utimensat				280
%define SYS_epoll_pwait			281
%define SYS_signalfd				282
%define SYS_timerfd_create			283
%define SYS_eventfd				284
%define SYS_fallocate				285
%define SYS_timerfd_settime			286
%define SYS_timerfd_gettime			287
%define SYS_accept4				288
%define SYS_signalfd4				289
%define SYS_eventfd2				290
%define SYS_epoll_create1			291
%define SYS_dup3				292
%define SYS_pipe2				293
%define SYS_inotify_init1			294
%define SYS_preadv				295
%define SYS_pwritev				296
%define SYS_rt_tgsigqueueinfo			297
%define SYS_perf_event_open			298
%define SYS_recvmmsg				299

%else

; ---------------------------------
; SysCall Macro
; ---------------------------------

%imacro syscall 1-6
 %if %0 > 1
  %if %0 > 2
   %if %0 > 3
    %if %0 > 4
     %if %0 > 5
      mov edi, %{6}
     %endif
     mov esi, %{5}
    %endif
    mov edx, %{4}
   %endif
   mov ecx, %{3}
  %endif
  mov ebx, %{2}
 %endif
 mov eax, SYS_ %+ %{1}
 int 0x80
%endmacro

; ---------------------------------
; System Call Listing
; ---------------------------------

SYS_restart_syscall    equ 0           ;old setup() system call
SYS_exit               equ 1
SYS_fork               equ 2
SYS_read               equ 3
SYS_write              equ 4
SYS_open               equ 5
SYS_close              equ 6
SYS_waitpid            equ 7
SYS_creat              equ 8
SYS_link               equ 9
SYS_unlink             equ 10
SYS_execve             equ 11
SYS_chdir              equ 12
SYS_time               equ 13
SYS_mknod              equ 14
SYS_chmod              equ 15
SYS_lchown16           equ 16
SYS_break              equ 17          ;old break syscall holder
SYS_stat               equ 18
SYS_lseek              equ 19
SYS_getpid             equ 20
SYS_mount              equ 21
SYS_oldumount          equ 22
SYS_setuid16           equ 23
SYS_getuid16           equ 24
SYS_stime              equ 25
SYS_ptrace             equ 26
SYS_alarm              equ 27
SYS_fstat              equ 28
SYS_pause              equ 29
SYS_utime              equ 30
SYS_stty               equ 31          ;old stty syscall holder
SYS_gtty               equ 32          ;old gtty syscall holder
SYS_access             equ 33
SYS_nice               equ 34
SYS_ftime              equ 35          ;old ftime syscall holder
SYS_sync               equ 36
SYS_kill               equ 37
SYS_rename             equ 38
SYS_mkdir              equ 39
SYS_rmdir              equ 40
SYS_dup                equ 41
SYS_pipe               equ 42
SYS_times              equ 43
SYS_prof               equ 44          ;old prof syscall holder
SYS_brk                equ 45
SYS_setgid16           equ 46
SYS_getgid16           equ 47
SYS_signal             equ 48
SYS_geteuid16          equ 49
SYS_getegid16          equ 50
SYS_acct               equ 51
SYS_umount             equ 52          ;recycled never used phys()
SYS_lock               equ 53          ;old lock syscall holder
SYS_ioctl              equ 54
SYS_fcntl              equ 55
SYS_mpx                equ 56          ;old mpx syscall holder
SYS_setpgid            equ 57
SYS_ulimit             equ 58          ;old ulimit syscall holder
SYS_olduname           equ 59
SYS_umask              equ 60
SYS_chroot             equ 61
SYS_ustat              equ 62
SYS_dup2               equ 63
SYS_getppid            equ 64
SYS_getpgrp            equ 65
SYS_setsid             equ 66
SYS_sigaction          equ 67
SYS_sgetmask           equ 68
SYS_ssetmask           equ 69
SYS_setreuid16         equ 70
SYS_setregid16         equ 71
SYS_sigsuspend         equ 72
SYS_sigpending         equ 73
SYS_sethostname        equ 74
SYS_setrlimit          equ 75
SYS_old_getrlimit      equ 76
SYS_getrusage          equ 77
SYS_gettimeofday       equ 78
SYS_settimeofday       equ 79
SYS_getgroups16        equ 80
SYS_setgroups16        equ 81
SYS_old_select         equ 82
SYS_symlink            equ 83
SYS_lstat              equ 84
SYS_readlink           equ 85
SYS_uselib             equ 86
SYS_swapon             equ 87
SYS_reboot             equ 88
SYS_old_readdir        equ 89
SYS_readdir            equ 89
SYS_old_mmap           equ 90
SYS_mmap               equ 90
SYS_munmap             equ 91
SYS_truncate           equ 92
SYS_ftruncate          equ 93
SYS_fchmod             equ 94
SYS_fchown16           equ 95
SYS_getpriority        equ 96
SYS_setpriority        equ 97
SYS_profil             equ 98          ;old profil syscall holder
SYS_statfs             equ 99
SYS_fstatfs            equ 100
SYS_ioperm             equ 101
SYS_socketcall         equ 102
SYS_syslog             equ 103
SYS_setitimer          equ 104
SYS_getitimer          equ 105
SYS_newstat            equ 106
SYS_newlstat           equ 107
SYS_newfstat           equ 108
SYS_uname              equ 109
SYS_iopl               equ 110
SYS_vhangup            equ 111
SYS_idle               equ 112         ;old idle syscall
SYS_vm86old            equ 113
SYS_wait4              equ 114
SYS_swapoff            equ 115
SYS_sysinfo            equ 116
SYS_ipc                equ 117
SYS_fsync              equ 118
SYS_sigreturn          equ 119
SYS_clone              equ 120
SYS_setdomainname      equ 121
SYS_newuname           equ 122
SYS_modify_ldt         equ 123
SYS_adjtimex           equ 124
SYS_mprotect           equ 125
SYS_sigprocmask        equ 126
SYS_create_module      equ 127         ;old create_module syscall
SYS_init_module        equ 128
SYS_delete_module      equ 129
SYS_get_kernel_syms    equ 130         ;old get_kernel_syms
SYS_quotactl           equ 131
SYS_getpgid            equ 132
SYS_fchdir             equ 133
SYS_bdflush            equ 134
SYS_sysfs              equ 135
SYS_personality        equ 136
SYS_afs_syscall        equ 137         ;reserved for andrew file system syscall
SYS_setfsuid16         equ 138
SYS_setfsgid16         equ 139
SYS_llseek             equ 140
SYS_getdents           equ 141
SYS_select             equ 142
SYS_flock              equ 143
SYS_msync              equ 144
SYS_readv              equ 145
SYS_writev             equ 146
SYS_getsid             equ 147
SYS_fdatasync          equ 148
SYS_sysctl             equ 149
SYS_mlock              equ 150
SYS_munlock            equ 151
SYS_mlockall           equ 152
SYS_munlockall         equ 153
SYS_sched_setparam     equ 154
SYS_sched_getparam     equ 155
SYS_sched_setscheduler equ 156
SYS_sched_getscheduler equ 157
SYS_sched_yield        equ 158
SYS_sched_get_priority_max    equ 159
SYS_sched_get_priority_min    equ 160
SYS_sched_rr_get_interval     equ 161
SYS_nanosleep          equ 162
SYS_mremap             equ 163
SYS_setresuid16        equ 164
SYS_getresuid16        equ 165
SYS_vm86               equ 166
SYS_query_module       equ 167         ;old SYS_query_module
SYS_poll               equ 168
SYS_nfsservctl         equ 169
SYS_setresgid16        equ 170
SYS_getresgid16        equ 171
SYS_prctl              equ 172
SYS_rt_sigreturn       equ 173
SYS_rt_sigaction       equ 174
SYS_rt_sigprocmask     equ 175
SYS_rt_sigpending      equ 176
SYS_rt_sigtimedwait    equ 177
SYS_rt_sigqueueinfo    equ 178
SYS_rt_sigsuspend      equ 179
SYS_pread64            equ 180
SYS_pwrite64           equ 181
SYS_chown16            equ 182
SYS_getcwd             equ 183
SYS_capget             equ 184
SYS_capset             equ 185
SYS_sigalstack         equ 186
SYS_sendfile           equ 187
SYS_getpmsg            equ 188         ;reserved for streams1
SYS_putpmsg            equ 189         ;reserved for streams2
SYS_vfork              equ 190
SYS_getrlimit          equ 191
SYS_mmap_pgoff         equ 192
SYS_truncate64         equ 193
SYS_ftruncate64        equ 194
SYS_stat64             equ 195
SYS_lstat64            equ 196
SYS_fstat64            equ 197
SYS_lchown             equ 198
SYS_getuid             equ 199
SYS_getgid             equ 200
SYS_geteuid            equ 201
SYS_getegid            equ 202
SYS_setreuid           equ 203
SYS_setregid           equ 204
SYS_getgroups          equ 205
SYS_setgroups          equ 206
SYS_fchown             equ 207
SYS_setresuid          equ 208
SYS_getresuid          equ 209
SYS_setresgid          equ 210
SYS_getresgid          equ 211
SYS_chown              equ 212
SYS_setuid             equ 213
SYS_setgid             equ 214
SYS_setfsuid           equ 215
SYS_setfsgid           equ 216
SYS_pivot_root         equ 217
SYS_mincore            equ 218
SYS_madvise            equ 219
SYS_getdents64         equ 220
SYS_fcntl64            equ 221
;SYS_ni_syscall        equ 222         ;reserved for TUX
;SYS_ni_syscall        equ 223
SYS_gettid             equ 224
SYS_readahead          equ 225
SYS_setxattr           equ 226
SYS_lsetxattr          equ 227
SYS_fsetxattr          equ 228
SYS_getxattr           equ 229
SYS_lgetxattr          equ 230
SYS_fgetxattr          equ 231
SYS_listxattr          equ 232
SYS_llistxattr         equ 233
SYS_flistxattr         equ 234
SYS_removexattr        equ 235
SYS_lremovexattr       equ 236
SYS_fremovexattr       equ 237
SYS_tkill              equ 238
SYS_sendfile64         equ 239
SYS_futex              equ 240
SYS_sched_set_affinity equ 241
SYS_sched_get_affinity equ 242
SYS_set_thread_area    equ 243
SYS_get_thread_area    equ 244
SYS_io_setup           equ 245
SYS_io_destroy         equ 246
SYS_io_getevents       equ 247
SYS_io_submit          equ 248
SYS_io_cancel          equ 249
SYS_fadvise64          equ 250
;SYS_ni_syscall        equ 251
SYS_exit_group         equ 252
SYS_lookup_dcookie     equ 253
SYS_epoll_create       equ 254
SYS_epoll_ctl          equ 255
SYS_epoll_wait         equ 256
SYS_remap_file_pages   equ 257
SYS_set_tid_address    equ 258
SYS_timer_create       equ 259
SYS_timer_settime      equ 260
SYS_timer_gettime      equ 261
SYS_timer_getoverrun   equ 262
SYS_timer_delete       equ 263
SYS_clock_settime      equ 264
SYS_clock_gettime      equ 265
SYS_clock_getres       equ 266
SYS_clock_nanosleep    equ 267
SYS_statfs64           equ 268
SYS_fstatfs64          equ 269
SYS_tgkill             equ 270
SYS_utimes             equ 271
SYS_fadvise64_64       equ 272
SYS_vserver            equ 273
SYS_mbind              equ 274
SYS_get_mempolicy      equ 275
SYS_set_mempolicy      equ 276
SYS_mq_open            equ 277
SYS_mq_unlink          equ 278
SYS_mq_timedsend       equ 279
SYS_mq_timedreceive    equ 280
SYS_mq_notify          equ 281
SYS_mq_getsetattr      equ 282
SYS_kexec_load         equ 283
SYS_waitid             equ 284
;SYS_ni_syscall        equ 285         ;available
SYS_add_key            equ 286
SYS_request_key        equ 287
SYS_keyctl             equ 288
SYS_ioprio_set         equ 289
SYS_ioprio_get         equ 290
SYS_inotify_init       equ 291
SYS_inotify_add_watch  equ 292
SYS_inotify_rm_watch   equ 293
SYS_migrate_pages      equ 294
SYS_openat             equ 295
SYS_mkdirat            equ 296
SYS_mknodat            equ 297
SYS_fchownat           equ 298
SYS_futimesat          equ 299
SYS_fstatat64          equ 300
SYS_unlinkat           equ 301
SYS_renameat           equ 302
SYS_linkat             equ 303
SYS_symlinkat          equ 304
SYS_readlinkat         equ 305
SYS_fchmodat           equ 306
SYS_faccessat          equ 307
SYS_pselect6           equ 308
SYS_ppoll              equ 309
SYS_unshare            equ 310
SYS_set_robust_list    equ 311
SYS_get_robust_list    equ 312
SYS_splice             equ 313
SYS_sync_file_range    equ 314
SYS_tee                equ 315
SYS_vmsplice           equ 316
SYS_move_pages         equ 317
SYS_getcpu             equ 318
SYS_epoll_pwait        equ 319
SYS_utimensat          equ 320
SYS_signalfd           equ 321
SYS_timerfd_create     equ 322
SYS_eventfd            equ 323
SYS_fallocate          equ 324
SYS_timerfd_settime    equ 325
SYS_timerfd_gettime    equ 326
SYS_signalfd4          equ 327
SYS_eventfd2           equ 328
SYS_epoll_create1      equ 329
SYS_dup3               equ 330
SYS_pipe2              equ 331
SYS_inotify_init1      equ 332
SYS_preadv             equ 333
SYS_pwritev            equ 334
SYS_rt_tgsigqueueinfo  equ 335
SYS_perf_event_open    equ 336
SYS_recvmmsg           equ 337
SYS_fanotify_event     equ 338
SYS_fanotify_mark      equ 339
SYS_prlimit64          equ 340

%endif ; __NASMX_ABI_BITS__

%endif ; __NASMX_SYSCALL_INC__
