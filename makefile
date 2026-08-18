ccf := -m32
ccf += -std=c89

ccf += -Wno-implicit-int
ccf += -Wno-implicit-function-declaration
ccf += -Wno-builtin-declaration-mismatch

ldf = -m32 -no-pie

srcd = src
scrd = scripts

srcc = $(wildcard $(srcd)/*.c)
srcs = $(wildcard $(srcd)/*.s)
src = $(srcc) $(srcs)

objd = obj
obj = $(srcc:$(srcd)/%.c=$(objd)/%.o) $(srcs:$(srcd)/%.s=$(objd)/%.o)

out = rc

all: $(out)

$(out): $(obj)
	cc $^ -o $@ $(ldf)

# esta parte >/dev/null 2>&1 para pipear less mas limpio
debug:
	@cc $(src) -o $@ $(ccf) $(ldf) -DDEBUG
	@./debug examples/test.ru 2>&1 >/dev/null
	@rm debug

install:
	cp $(out) /usr/bin

clean: $(objd)
	rm -r $<

$(objd):
	mkdir $(objd)

$(objd)/%.o: $(srcd)/%.c | $(objd)
	cc -c $< -o $@ $(ccf)

$(objd)/%.o: $(srcd)/%.s | $(objd)
	cc -c $< -o $@ $(ccf)
