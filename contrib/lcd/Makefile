all: 
	@ls *.char | while read i; do echo -n "$$i: "; ruby ./makechar.rb -format=ecmd $$i; done
