PROGS = condoorb.py
CONFS = oorb.2body.conf oorb.nbody.conf
MOPS_BIN = ${MOPS_HOME}/bin
MOPS_CONF = ${MOPS_HOME}/config

dummy:

install:
	mkdir -p $(MOPS_CONF)/oorb
	chmod g+w $(MOPS_CONF)/oorb
	/usr/bin/install $(PROGS) $(MOPS_BIN)
	/usr/bin/install $(CONFS) $(MOPS_CONF)/oorb

