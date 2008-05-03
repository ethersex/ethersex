TOPDIR := ..
HEADER=header.tk
TAIL=tail.tk

kconfig.tk: ${TOPDIR}/Makefile ${TOPDIR}/config.in \
		tkparse ${HEADER} ${TAIL}
	@if [ -f /usr/local/bin/wish ];	then \
		echo '#!'"/usr/local/bin/wish -f" > kconfig.tk; \
	else \
		echo '#!'"/usr/bin/wish -f" > kconfig.tk; \
	fi
	cat ${HEADER} >> ./kconfig.tk
	./tkparse < ${TOPDIR}/config.in >> kconfig.tk
	echo "set defaults \"defconfig\"" >> kconfig.tk
	cat ${TAIL} >> kconfig.tk
	chmod 755 kconfig.tk

tkparse: tkparse.o tkcond.o tkgen.o
	${HOSTCC} -o tkparse tkparse.o tkcond.o tkgen.o

tkparse.o: tkparse.c tkparse.h

tkcond.o: tkcond.c tkparse.h

tkgen.o: tkgen.c tkparse.h

tkparse.o tkcond.o tkgen.o:
	$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $(@:.o=.c)

clean:
	rm -f *~ kconfig.tk *.o tkparse

