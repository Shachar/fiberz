lib_LTLIBRARIES=libfiberz.la

AM_CPPFLAGS=-I$(top_srcdir)/include
AM_CXXFLAGS=-std=c++20

libfiberz_la_SOURCES=fiber.cpp reactor.cpp unique_mmap.cpp context.cpp
libfiberz_la_SOURCES+=platform/$(host_cpu)/arch_context.cpp platform/$(host_cpu)/context_asm.s
libfiberz_la_SOURCES+=internal/cascaded_time_queue.cpp
libfiberz_la_SOURCES+=utility/excontext.cpp
