BLDDIR=build
OBJDIR=obj
SRCDIR=src
EXMDIR=examples
CFLAGS=-Wall -Wswitch-enum -std=c99 -O2 -s
ARFLAGS=crs
LIBS=


$(OBJDIR)/AMkd.o: $(SRCDIR)/AMkd.c | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/example.o: $(EXMDIR)/example.c $(BLDDIR)/libAMkd.a | $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)


$(BLDDIR)/libAMkd.a: $(OBJDIR)/AMkd.o | $(BLDDIR)
	$(AR) $(ARFLAGS) $@ $<

$(BLDDIR)/example: $(OBJDIR)/example.o $(BLDDIR)/libAMkd.a | $(BLDDIR)
	$(CC) -o $@ $< $(CFLAGS) $(LIBS) -L$(BLDDIR) -l:libAMkd.a

$(BLDDIR):
	mkdir $(BLDDIR)


.PHONY: clean library example

clean:
	$(RM) $(OBJDIR)/*.o $(BLDDIR)/*

library: $(BLDDIR)/libAMkd.a ;

example: $(BLDDIR)/example ;
