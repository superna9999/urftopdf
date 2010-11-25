all: urftopdf

urftopdf:urftopdf.c
	$(CC) urftopdf.c -lhpdf -lcups -o urftopdf -lm $(CFLAGS)

install:urftopdf
	DESTDIR=$(DESTDIR) ./install_pdf.sh

clean:
	rm urftopdf
