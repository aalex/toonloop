all:
	python setup.py build
	help2man --no-info --include=man_toonloop.txt --name="The Toonloop Live Stop Motion Tool" ./toonloop > toonloop.1 
	@echo And now:
	@echo sudo make install

toonloop.1: all
	@echo DONE BUILDING

install: toonloop.1
	python setup.py install --prefix=/usr/local
	install toonloop.desktop /usr/local/share/applications/
	convert -geometry 48x48 -background none toonloop.svg toonloop.png
	install toonloop.png /usr/local/share/icons/hicolor/48x48/apps/
	install toonloop.svg /usr/local/share/icons/
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
	trial rats/test toon/test

deb:
	dpkg-buildpackage -r

html:
	mkdir -p doc
	epydoc --html --output=doc --verbose --show-imports --name=Toonloop  --url=http://toonloop.com/static/doc/ toon rats
