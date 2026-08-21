as = cc
cc = cc
ld = cc

asf := -m32

ccf := -m32
ccf += -std=c89

ccf += -Wall
ccf += -Wextra
ccf += -Wpedantic
ccf += -Werror

ccf += -Wno-unused-parameter
ccf += -Wno-unused-variable
ccf += -Wno-return-type
ccf += -Wno-implicit-int
ccf += -Wno-implicit-function-declaration
ccf += -Wno-builtin-declaration-mismatch

ldf := -m32
ldf += -no-pie
ldf += -nostartfiles

srcd = src

srcs = $(wildcard $(srcd)/*.s)
srcc = $(wildcard $(srcd)/*.c)
src = $(srcs) $(srcc)

objd = obj
obj = $(srcs:$(srcd)/%.s=$(objd)/%.o) $(srcc:$(srcd)/%.c=$(objd)/%.o)

out = rc
man = man.1

raw ?= 0
ifeq ($(raw), 1)
        hid =
        say = @:
else
        hid = @
        say = @echo
endif

.PHONY: all debug clean install install-syntax

all: $(out)

$(out): $(obj)
	$(say) -en '\e[1;33m  LD\t$@...'
	$(hid)$(ld) $^ -o $@ $(ldf)
	$(say) -e 'OK\e[0m'

debug:
	@cc $(src) -o $@ $(ccf) $(ldf) -DDEBUG
	@./$@ -silent -debug $(f) 2>&1 | less -R
	@rm -f $@

install: $(out)
	cp $(out) /usr/bin
	cp $(man) /usr/share/man/man1/rc.1

install-syntax:
	cd syntax; ./install-syntax

clean:
	rm -rf $(objd)
	rm -f $(out)

$(objd):
	$(hid)mkdir $(objd)

$(objd)/%.o: $(srcd)/%.s | $(objd)
	$(say) -en '\e[1;36m  AS\t$@...'
	$(hid)$(as) -c $< -o $@ $(asf)
	$(say) -e 'OK\e[0m'

$(objd)/%.o: $(srcd)/%.c | $(objd)
	$(say) -en '\e[1;32m  CC\t$@...'
	$(hid)$(cc) -c $< -o $@ $(ccf)
	$(say) -e 'OK\e[0m'
