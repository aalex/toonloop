all:
	python setup.py build
	@echo And now:
	@echo sudo make install
install:
	python setup.py install --prefix=/usr/local
	install Toonloop.desktop /usr/local/share/applications/Toonloop.desktop
	install Toonloop.svg /usr/local/share/icons/Toonloop.svg
	help2man -N -i manpage_inc.txt -n "The ToonLoop Live Stop Motion Tool" ./toonloop > toonloop.1 
	install -D toonloop.1 /usr/local/share/man/man1/toonloop.1

dist:
	python setup.py sdist 

uninstall:
	@echo uninstall can only be used when using develop mode
clean:
	@echo "You might to run make clean as root. Try sudo make clean."
	rm -rf toonloop.1 build dist toonloop.egg-info
	find . -name \*.pyc  -exec rm {} \;
	find . -name _trial_temp -exec rm -rf {} \;
	@echo DONE

check:
	trial rats/test

