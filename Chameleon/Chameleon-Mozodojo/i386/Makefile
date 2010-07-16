
#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

CFLAGS	= -O $(MORECPP) -arch i386 -g -static
DEFINES=
CONFIG = hd
LIBDIR = libsa
INC = -I. -I$(LIBDIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld

#
# these paths are only valid in subdirectories of this directory
#
OBJROOT=`pwd`/../../obj/i386
SYMROOT=`pwd`/../../sym/i386
DSTROOT=`pwd`/../../dst/i386
SRCROOT=/tmp

VPATH = $(OBJROOT):$(SYMROOT)

# The order of building is important.
SUBDIRS = util libsa libsaio boot2 boot1 boot0 cdboot

all embedtheme tags debug install installhdrs:
	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done

installsrc:
	tar cf - . | (cd ${SRCROOT}; tar xfBp -)
