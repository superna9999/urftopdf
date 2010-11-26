all: urftopdf urftourf

urftopdf:urftopdf.c unirast.h
	$(CC) urftopdf.c -lhpdf -lcups -o urftopdf -lm $(CFLAGS)

urftourf:urftourf.c unirast.h
	$(CC) urftourf.c -o urftourf $(CFLAGS)

install:urftopdf
	DESTDIR=$(DESTDIR) ./install_pdf.sh

clean:
	-rm urftopdf urftourf
