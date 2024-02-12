LINK_TARGET1 = client_music
LINK_TARGET2 = server_music
LINK_TARGET3 = udp_music
LINK_TARGET4 = ip_over_music
OBJS1 = \
	client_music.o
OBJS2 = \
	server_music.o	
OBJS3 = \
	udp_music.o
OBJS4 = \
	ip_over_music.o
REBUILDABLES = $(OBJS1) $(LINK_TARGET1) $(OBJS2) $(LINK_TARGET2) $(OBJS3) $(LINK_TARGET3) $(OBJS4) $(LINK_TARGET4)
all : $(LINK_TARGET1) $(LINK_TARGET2) $(LINK_TARGET3) 
clean: 
	rm -f $(REBUILDABLES)
$(LINK_TARGET1) : $(OBJS1)
	cc -g -o $@ $^ -lm -lpulse-simple -lpulse
$(LINK_TARGET2) : $(OBJS2)
	cc -g -o $@ $^ -lm -lpulse-simple -lpulse
$(LINK_TARGET3) : $(OBJS3)
	cc -g -o $@ $^ -lm -lpulse-simple -lpulse
$(LINK_TARGET4) : $(OBJS4)
	cc -g -o $@ $^ -lm -lpulse-simple -lpulse
%.o : %.c
	cc -g  -Wall -o $@ -c $< -lm -lpulse-simple -lpulse