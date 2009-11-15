all:
	python setup.py build
	@echo And now:
	@echo sudo make install
	help2man --no-info --include=manpage_inc.txt --name="The Toonloop Live Stop Motion Tool" ./toonloop > toonloop.1 
toonloop.1: all
	@echo DONE BUILDING
install: toonloop.1
	python setup.py install --prefix=/usr/local
	install Toonloop.desktop /usr/local/share/applications/Toonloop.desktop
	install Toonloop.svg /usr/local/share/icons/Toonloop.svg
	install -D toonloop.1 /usr/local/share/man/man1/toonloop.1
	@echo DONE INSTALLING

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

