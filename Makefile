## Hanged Bunny Studio Library ------    ---  -- --- -    ---  --    - -- -   -
## Hanged Bunny Studio 2014-2016 ----- -- - -- -  -  -- -- - -- --  -- -- -- --
## ----------------------------------- -- -    - - - -- -- - -- ---- -    -- --
## ----------------------------------    -- -- - --- -    ---  --   -- -- -   -
## ----------------------------------------------------------------------------

## ----------------------------------------------------------------------------
## Configuration --------------------------------------------------------------

  MOD		=	gendiploma
  BIN		=	gendiploma
  CFLAGS	?=	-O2 -g
  CFLAGS	+=	-W -Wall -pipe

  CPPFLAGS	+=	-I./include/ -DGENDIPLOMA_DATADIR=\"$(DATADIR)\"
  CSTD		=	-std=c11
  PREFIX	?=	/usr/local
  BINDIR	?=	$(PREFIX)/bin
  DATADIR	?=	$(PREFIX)/share/gendiploma
  DESTDIR	?=
  SRC		=	$(wildcard src/*.c)
  OBJ		=	$(SRC:.c=.o)
  LIBPATH	=	-L${HOME}/.froot/lib/
  LIB		=	-llapin -lsfml-graphics -lsfml-audio -lsfml-window	\
			-lsfml-system -lstdc++ -lm -ldl -lpthread -lavcall

  ## Rules ------------------------------------------------------------------
  all:		bin
  bin:		$(OBJ)
		@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $(BIN) $(LIBPATH) $(LIB)
		@echo "[OUT] " $(BIN)
		@echo $(BIN) | tr '[:lower:]' '[:upper:]'
  .c.o:
		@$(CC) $(CPPFLAGS) $(CSTD) $(CFLAGS) -c $< -o $@
		@echo "[GCC]" $<
  clean:
		@rm -f $(OBJ)
  fclean:	clean
		@rm -f $(BIN)
  re:		fclean all
  install: all
		install -d $(DESTDIR)$(BINDIR)
		install -m 0755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

## ----------------------------------------------------------------------------
## MISC -----------------------------------------------------------------------

  .PHONY: all clean fclean re install

  .SUFFIXES: .cpp .o
