all: urftopdf

# Setup the libharu path here
HARU=libharu-2.2.1
HARU_LIB=$(HARU)/build/lib/
HARU_INCLUDE=$(HARU)/build/include

$(HARU_INCLUDE)/hpdf.h:
	cd $(HARU) && ./configure --prefix=$(PWD)/$(HARU)/build --enable-shared=no --enable-static=yes && make && make install

urftopdf:urftopdf.c $(HARU_INCLUDE)/hpdf.h
	$(CC) urftopdf.c -lhpdf -lcups -o urftopdf -I$(HARU_INCLUDE) -L$(HARU_LIB) -lm

clean:
	rm urftopdf

cleanharu:
	rm -fr $(HARU)/build
