OBJS+= hosted.o

CFLAGS+= -O0 -ggdb -DSUPPORTS_POSIX_TIMERS
CFLAGS+= -D TICKRATE=4000000 -D_POSIX_C_SOURCE=199309L -D_GNU_SOURCE
#CFLAGS+= -fsanitize=undefined -fsanitize=address -pthread
CFLAGS+= -pthread

LDFLAGS+= -lrt

run: ${OBJDIR}/viaems
	${OBJDIR}/viaems
