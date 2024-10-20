all: psyq470

psyq470: Psy-Q_47.zip
	mkdir -p $@
	cd $@ && unzip ../$^
Psy-Q_47.zip:
	wget https://psxdev.net/downloads/$@
	sha256sum -c $@.sha256
