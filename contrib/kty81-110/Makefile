BINS = kty-merge nc_udp kty-calibrate
all: $(BINS)

clean:
	rm -f $(BINS)

install: $(BINS)
	@for i in $(BINS); do echo cp $$i /usr/local/bin; cp $$i /usr/local/bin; done
