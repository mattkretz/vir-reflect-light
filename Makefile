CXXFLAGS=-O2 -std=c++20 -g0
CXXFLAGS+=-Wall -Wextra -Wpedantic
CXXFLAGS+=-I$(PWD)

prefix=/usr/local
includedir=$(prefix)/include

all:
	@echo "No library to build."
	@echo "- verify everything works as expected with 'make check'."
	@echo "- install with 'make install prefix=<install prefix>'."

install:
	install -d $(includedir)/vir
	install -m 644 -t $(includedir)/vir vir/*.h

.PHONY: check
check: test.o
	./check-result.sh

test.o: test.cpp vir/*.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.PHONY: help
help:
	@echo "all"
	@echo "install"
	@echo "check"
	@echo "clean"

.PHONY: clean
clean:
	rm -f test.o
