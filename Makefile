BLDDIR=build
OBJDIR=obj
SRCDIR=src
EXMDIR=examples
CFLAGS=-Wall -std=c99 -O2 -s
ARFLAGS=crs
LIBS=


$(OBJDIR)/AMkd.o: $(SRCDIR)/AMkd.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/example0.o: $(EXMDIR)/example0.c $(BLDDIR)/libAMkd.a | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/example1.o: $(EXMDIR)/example1.c $(BLDDIR)/libAMkd.a | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)


$(BLDDIR)/libAMkd.a: $(OBJDIR)/AMkd.o | $(BLDDIR)
	$(AR) $(ARFLAGS) $@ $<

$(BLDDIR)/example0: $(OBJDIR)/example0.o $(BLDDIR)/libAMkd.a | $(BLDDIR)
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) -L$(BLDDIR) -l:libAMkd.a

$(BLDDIR)/example1: $(OBJDIR)/example1.o $(BLDDIR)/libAMkd.a | $(BLDDIR)
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) -L$(BLDDIR) -l:libAMkd.a

$(BLDDIR):
	mkdir $(BLDDIR)


.PHONY: clean library example0 example1

clean:
	$(RM) $(OBJDIR)/*.o $(BLDDIR)/*

library: $(BLDDIR)/libAMkd.a ;

example0: $(BLDDIR)/example0 ;

example1: $(BLDDIR)/example1 ;
