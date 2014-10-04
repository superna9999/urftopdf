# PDF_BACKEND are: 
# - qpdf
# - hpdf
PDF_BACKEND ?= hpdf

ifeq ($(PDF_BACKEND), hpdf)
	FLAGS+=-DHPDF_BACKEND=1 -lhpdf -lm -lcups
endif
ifeq ($(PDF_BACKEND), qpdf)
	FLAGS+=-DQPDF_BACKEND=1 $(shell pkg-config --cflags --libs libqpdf)
endif
CXXFLAGS+=-Wall

all: urftopdf

urftopdf:urftopdf.cpp unirast.h
	$(CXX) urftopdf.cpp -o urftopdf $(CXXFLAGS) $(FLAGS)

install:urftopdf
	DESTDIR=$(DESTDIR) ./install_pdf.sh

clean:
	-rm urftopdf
