all:
	(cd src; make all)
	(cp src/*.o ./tests)
	(cd tests; make all)

clean:
	(cd src; make clean)
	(cd tests; make clean)
