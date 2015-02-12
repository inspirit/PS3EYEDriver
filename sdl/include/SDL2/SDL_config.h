/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SDL_config_h
#define _SDL_config_h

/**
 *  \file SDL_config.h.in
 *
 *  This is a set of defines to configure the SDL features
 */

/* General platform specific identifiers */
#include "SDL_platform.h"

/* C language features */
/* #undef const */
/* #undef inline */
/* #undef volatile */

/* C datatypes */
/* Define SIZEOF_VOIDP for 64/32 architectures */
#ifdef __LP64__
#define SIZEOF_VOIDP 8
#else
#define SIZEOF_VOIDP 4
#endif

#define HAVE_GCC_ATOMICS 1
/* #undef HAVE_GCC_SYNC_LOCK_TEST_AND_SET */
/* #undef HAVE_PTHREAD_SPINLOCK */

/* Comment this if you want to build without any C library requirements */
#define HAVE_LIBC 1
#if HAVE_LIBC

/* Useful headers */
/* #undef HAVE_ALLOCA_H */
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDIO_H 1
/* #undef STDC_HEADERS */
#define HAVE_STDLIB_H 1
#define HAVE_STDARG_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MATH_H 1
/* #undef HAVE_ICONV_H */
#define HAVE_SIGNAL_H 1
/* #undef HAVE_ALTIVEC_H */
/* #undef HAVE_PTHREAD_NP_H */
/* #undef HAVE_LIBUDEV_H */
/* #undef HAVE_DBUS_DBUS_H */

/* C library functions */
#define HAVE_MALLOC 1
#define HAVE_CALLOC 1
#define HAVE_REALLOC 1
#define HAVE_FREE 1
/* #undef HAVE_ALLOCA */
#ifndef __WIN32__ /* Don't use C runtime versions of these on Windows */
#define HAVE_GETENV 1
/* #undef HAVE_SETENV */
#define HAVE_PUTENV 1
/* #undef HAVE_UNSETENV */
#endif
#define HAVE_QSORT 1
#define HAVE_ABS 1
/* #undef HAVE_BCOPY */
#define HAVE_MEMSET 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMCMP 1
#define HAVE_STRLEN 1
/* #undef HAVE_STRLCPY */
/* #undef HAVE_STRLCAT */
#define HAVE_STRDUP 1
#define HAVE__STRREV 1
#define HAVE__STRUPR 1
#define HAVE__STRLWR 1
/* #undef HAVE_INDEX */
/* #undef HAVE_RINDEX */
#define HAVE_STRCHR 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_ITOA 1
#define HAVE__LTOA 1
/* #undef HAVE__UITOA */
#define HAVE__ULTOA 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE__I64TOA 1
#define HAVE__UI64TOA 1
#define HAVE_STRTOLL 1
#define HAVE_STRTOULL 1
#define HAVE_STRTOD 1
#define HAVE_ATOI 1
#define HAVE_ATOF 1
#define HAVE_STRCMP 1
#define HAVE_STRNCMP 1
#define HAVE__STRICMP 1
#define HAVE_STRCASECMP 1
#define HAVE__STRNICMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_VSSCANF 1
#define HAVE_VSNPRINTF 1
#define HAVE_M_PI 1
#define HAVE_ATAN 1
#define HAVE_ATAN2 1
/* #undef HAVE_ACOS */
/* #undef HAVE_ASIN */
#define HAVE_CEIL 1
#define HAVE_COPYSIGN 1
#define HAVE_COS 1
#define HAVE_COSF 1
#define HAVE_FABS 1
#define HAVE_FLOOR 1
#define HAVE_LOG 1
#define HAVE_POW 1
#define HAVE_SCALBN 1
#define HAVE_SIN 1
#define HAVE_SINF 1
#define HAVE_SQRT 1
#define HAVE_FSEEKO 1
#define HAVE_FSEEKO64 1
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SA_SIGACTION */
#define HAVE_SETJMP 1
#define HAVE_NANOSLEEP 1
/* #undef HAVE_SYSCONF */
/* #undef HAVE_SYSCTLBYNAME */
/* #undef HAVE_CLOCK_GETTIME */
/* #undef HAVE_GETPAGESIZE */
/* #undef HAVE_MPROTECT */
/* #undef HAVE_ICONV */
/* #undef HAVE_PTHREAD_SETNAME_NP */
/* #undef HAVE_PTHREAD_SET_NAME_NP */
/* #undef HAVE_SEM_TIMEDWAIT */
#elif __WIN32__
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#else
/* We may need some replacement for stdarg.h here */
#include <stdarg.h>
#endif /* HAVE_LIBC */

/* SDL internal assertion support */
/* #undef SDL_DEFAULT_ASSERT_LEVEL */

/* Allow disabling of core subsystems */
/* #undef SDL_ATOMIC_DISABLED */
/* #undef SDL_AUDIO_DISABLED */
/* #undef SDL_CPUINFO_DISABLED */
/* #undef SDL_EVENTS_DISABLED */
/* #undef SDL_FILE_DISABLED */
/* #undef SDL_JOYSTICK_DISABLED */
#define SDL_HAPTIC_DISABLED 1
/* #undef SDL_LOADSO_DISABLED */
/* #undef SDL_RENDER_DISABLED */
/* #undef SDL_THREADS_DISABLED */
/* #undef SDL_TIMERS_DISABLED */
/* #undef SDL_VIDEO_DISABLED */
/* #undef SDL_POWER_DISABLED */
/* #undef SDL_FILESYSTEM_DISABLED */

/* Enable various audio drivers */
/* #undef SDL_AUDIO_DRIVER_ALSA */
/* #undef SDL_AUDIO_DRIVER_ALSA_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_ARTS */
/* #undef SDL_AUDIO_DRIVER_ARTS_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_PULSEAUDIO */
/* #undef SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_HAIKU */
/* #undef SDL_AUDIO_DRIVER_BSD */
/* #undef SDL_AUDIO_DRIVER_COREAUDIO */
#define SDL_AUDIO_DRIVER_DISK 1
#define SDL_AUDIO_DRIVER_DUMMY 1
/* #undef SDL_AUDIO_DRIVER_XAUDIO2 */
/* #undef SDL_AUDIO_DRIVER_DSOUND */
/* #undef SDL_AUDIO_DRIVER_ESD */
/* #undef SDL_AUDIO_DRIVER_ESD_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_NAS */
/* #undef SDL_AUDIO_DRIVER_NAS_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_SNDIO */
/* #undef SDL_AUDIO_DRIVER_SNDIO_DYNAMIC */
/* #undef SDL_AUDIO_DRIVER_OSS */
/* #undef SDL_AUDIO_DRIVER_OSS_SOUNDCARD_H */
/* #undef SDL_AUDIO_DRIVER_PAUDIO */
/* #undef SDL_AUDIO_DRIVER_QSA */
/* #undef SDL_AUDIO_DRIVER_SUNAUDIO */
#define SDL_AUDIO_DRIVER_WINMM 1
/* #undef SDL_AUDIO_DRIVER_FUSIONSOUND */
/* #undef SDL_AUDIO_DRIVER_FUSIONSOUND_DYNAMIC */

/* Enable various input drivers */
/* #undef SDL_INPUT_LINUXEV */
/* #undef SDL_INPUT_LINUXKD */
/* #undef SDL_INPUT_TSLIB */
/* #undef SDL_JOYSTICK_HAIKU */
/* #undef SDL_JOYSTICK_DINPUT */
/* #undef SDL_JOYSTICK_DUMMY */
/* #undef SDL_JOYSTICK_IOKIT */
/* #undef SDL_JOYSTICK_LINUX */
#define SDL_JOYSTICK_WINMM 1
/* #undef SDL_JOYSTICK_USBHID */
/* #undef SDL_JOYSTICK_USBHID_MACHINE_JOYSTICK_H */
/* #undef SDL_HAPTIC_DUMMY */
/* #undef SDL_HAPTIC_LINUX */
/* #undef SDL_HAPTIC_IOKIT */
/* #undef SDL_HAPTIC_DINPUT */

/* Enable various shared object loading systems */
/* #undef SDL_LOADSO_HAIKU */
/* #undef SDL_LOADSO_DLOPEN */
/* #undef SDL_LOADSO_DUMMY */
/* #undef SDL_LOADSO_LDG */
#define SDL_LOADSO_WINDOWS 1

/* Enable various threading systems */
/* #undef SDL_THREAD_PTHREAD */
/* #undef SDL_THREAD_PTHREAD_RECURSIVE_MUTEX */
/* #undef SDL_THREAD_PTHREAD_RECURSIVE_MUTEX_NP */
#define SDL_THREAD_WINDOWS 1

/* Enable various timer systems */
/* #undef SDL_TIMER_HAIKU */
/* #undef SDL_TIMER_DUMMY */
/* #undef SDL_TIMER_UNIX */
#define SDL_TIMER_WINDOWS 1
/* #undef SDL_TIMER_WINCE */

/* Enable various video drivers */
/* #undef SDL_VIDEO_DRIVER_HAIKU */
/* #undef SDL_VIDEO_DRIVER_COCOA */
/* #undef SDL_VIDEO_DRIVER_DIRECTFB */
/* #undef SDL_VIDEO_DRIVER_DIRECTFB_DYNAMIC */
#define SDL_VIDEO_DRIVER_DUMMY 1
#define SDL_VIDEO_DRIVER_WINDOWS 1
/* #undef SDL_VIDEO_DRIVER_WAYLAND */

#if 0
/* !!! FIXME: in configure script version, missing here: */
#undef SDL_VIDEO_DRIVER_WAYLAND_QT_TOUCH
#undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC
#undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_EGL
#undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_CURSOR
#undef SDL_VIDEO_DRIVER_WAYLAND_DYNAMIC_XKBCOMMON
#endif

/* #undef SDL_VIDEO_DRIVER_MIR */
/* #undef SDL_VIDEO_DRIVER_MIR_DYNAMIC */
/* #undef SDL_VIDEO_DRIVER_MIR_DYNAMIC_XKBCOMMON */
/* #undef SDL_VIDEO_DRIVER_X11 */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XEXT */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XCURSOR */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XINERAMA */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XINPUT2 */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XRANDR */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XSS */
/* #undef SDL_VIDEO_DRIVER_X11_DYNAMIC_XVIDMODE */
/* #undef SDL_VIDEO_DRIVER_X11_XCURSOR */
/* #undef SDL_VIDEO_DRIVER_X11_XINERAMA */
/* #undef SDL_VIDEO_DRIVER_X11_XINPUT2 */
/* #undef SDL_VIDEO_DRIVER_X11_XINPUT2_SUPPORTS_MULTITOUCH */
/* #undef SDL_VIDEO_DRIVER_X11_XRANDR */
/* #undef SDL_VIDEO_DRIVER_X11_XSCRNSAVER */
/* #undef SDL_VIDEO_DRIVER_X11_XSHAPE */
/* #undef SDL_VIDEO_DRIVER_X11_XVIDMODE */
/* #undef SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS */
/* #undef SDL_VIDEO_DRIVER_X11_CONST_PARAM_XDATA32 */
/* #undef SDL_VIDEO_DRIVER_X11_CONST_PARAM_XEXTADDDISPLAY */
/* #undef SDL_VIDEO_DRIVER_X11_HAS_XKBKEYCODETOKEYSYM */

/* #undef SDL_VIDEO_RENDER_D3D */
/* #undef SDL_VIDEO_RENDER_D3D11 */
#define SDL_VIDEO_RENDER_OGL 1
/* #undef SDL_VIDEO_RENDER_OGL_ES */
/* #undef SDL_VIDEO_RENDER_OGL_ES2 */
/* #undef SDL_VIDEO_RENDER_DIRECTFB */

/* Enable OpenGL support */
#define SDL_VIDEO_OPENGL 1
/* #undef SDL_VIDEO_OPENGL_ES */
/* #undef SDL_VIDEO_OPENGL_ES2 */
/* #undef SDL_VIDEO_OPENGL_BGL */
/* #undef SDL_VIDEO_OPENGL_CGL */
/* #undef SDL_VIDEO_OPENGL_GLX */
#define SDL_VIDEO_OPENGL_WGL 1
/* #undef SDL_VIDEO_OPENGL_EGL */
/* #undef SDL_VIDEO_OPENGL_OSMESA */
/* #undef SDL_VIDEO_OPENGL_OSMESA_DYNAMIC */

/* Enable system power support */
/* #undef SDL_POWER_LINUX */
#define SDL_POWER_WINDOWS 1
/* #undef SDL_POWER_MACOSX */
/* #undef SDL_POWER_HAIKU */
/* #undef SDL_POWER_HARDWIRED */

/* Enable system filesystem support */
/* #undef SDL_FILESYSTEM_HAIKU */
/* #undef SDL_FILESYSTEM_COCOA */
/* #undef SDL_FILESYSTEM_DUMMY */
/* #undef SDL_FILESYSTEM_UNIX */
#define SDL_FILESYSTEM_WINDOWS 1

/* Enable assembly routines */
#define SDL_ASSEMBLY_ROUTINES 1
/* #undef SDL_ALTIVEC_BLITTERS */


/* Platform specific definitions */
#if !defined(__WIN32__)
#  if !defined(_STDINT_H_) && !defined(_STDINT_H) && !defined(HAVE_STDINT_H) && !defined(_HAVE_STDINT_H)
typedef unsigned int size_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef unsigned long uintptr_t;
#  endif /* if (stdint.h isn't available) */
#else /* __WIN32__ */
#  if !defined(_STDINT_H_) && !defined(HAVE_STDINT_H) && !defined(_HAVE_STDINT_H)
#    if defined(__GNUC__) || defined(__DMC__) || defined(__WATCOMC__)
#define HAVE_STDINT_H	1
#    elif defined(_MSC_VER)
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#      ifndef _UINTPTR_T_DEFINED
#        ifdef  _WIN64
typedef unsigned __int64 uintptr_t;
#          else
typedef unsigned int uintptr_t;
#        endif
#define _UINTPTR_T_DEFINED
#      endif
/* Older Visual C++ headers don't have the Win64-compatible typedefs... */
#      if ((_MSC_VER <= 1200) && (!defined(DWORD_PTR)))
#define DWORD_PTR DWORD
#      endif
#      if ((_MSC_VER <= 1200) && (!defined(LONG_PTR)))
#define LONG_PTR LONG
#      endif
#    else /* !__GNUC__ && !_MSC_VER */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#      ifndef _SIZE_T_DEFINED_
#define _SIZE_T_DEFINED_
typedef unsigned int size_t;
#      endif
typedef unsigned int uintptr_t;
#    endif /* __GNUC__ || _MSC_VER */
#  endif /* !_STDINT_H_ && !HAVE_STDINT_H */
#endif /* __WIN32__ */

#endif /* _SDL_config_h */
