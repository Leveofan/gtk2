## Process this file with automake to produce Makefile.in
include $(top_srcdir)/Makefile.decl

libgdkincludedir = $(includedir)/gtk-3.0/gdk
libgdkwaylandincludedir = $(includedir)/gtk-3.0/gdk/wayland

AM_CPPFLAGS =					\
	-DG_LOG_DOMAIN=\"Gdk\"			\
	-DGDK_COMPILATION			\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/gdk			\
	-I$(top_builddir)/gdk			\
	$(GDK_HIDDEN_VISIBILITY_CFLAGS)		\
	$(GTK_DEBUG_FLAGS)			\
	$(GDK_DEP_CFLAGS)

LDADDS = $(GDK_DEP_LIBS)

noinst_LTLIBRARIES =				\
	libgdk-wayland.la

libgdk_wayland_la_SOURCES =			\
	gdkapplaunchcontext-wayland.c		\
	gdkcursor-wayland.c			\
	gdkdevice-wayland.c			\
	gdkdisplay-wayland.c			\
	gdkdisplay-wayland.h			\
	gdkdnd-wayland.c			\
	gdkeventsource.c			\
	gdkkeys-wayland.c			\
	gdkscreen-wayland.c			\
	gdkselection-wayland.c			\
	gdkwindow-wayland.c			\
	gdkwayland.h				\
	gdkprivate-wayland.h

libgdkinclude_HEADERS =				\
	gdkwayland.h

libgdkwaylandinclude_HEADERS = 			\
	gdkwaylanddevice.h			\
	gdkwaylanddisplay.h			\
	gdkwaylandselection.h			\
	gdkwaylandwindow.h

-include $(top_srcdir)/git.mk
