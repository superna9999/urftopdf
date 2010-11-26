all: urftopdf

urftopdf:urftopdf.c unirast.h
	$(CC) urftopdf.c -lhpdf -lcups -o urftopdf -lm $(CFLAGS)

install:urftopdf
	DESTDIR=$(DESTDIR) ./install_pdf.sh

clean:
	-rm urftopdf
