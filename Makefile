all:
	python setup.py build
	@echo And now:
	@echo sudo make install
install:
	python setup.py install --prefix=/usr/local
	install ToonLoop.svg /usr/local/share/applications/ToonLoop.desktop
	install ToonLoop.svg /usr/local/share/icons/ToonLoop.svg
	help2man -N -i manpage_inc.txt -n "The ToonLoop Live Stop Motion Tool" toonloop > toonloop.1 
	install toonloop.1 /usr/local/share/man/man1/toonloop.1

uninstall:
	@echo uninstall can only be used when using develop mode

	
