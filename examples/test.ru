word a[10];
word b = 30;
byte c;

pub fn main {
	word d = b + 70;
	word e[20];
	word f[2] = { 1, 2 };

	printf("d: %d\n", d);

	word l;
	for l = 0; l < 2; l = l + 1
		printf("f[%d]: %d\n", l, f[l]);

	word x = 10;
	if x > 5 {
		printf("10 is greater than 5\n");
	} else {
		printf("10 is not greater than 5\n");
	}

	word i = 0;
	while i < 3 {
		printf("i: %d\n", i);
		i = i + 1;
	}

	word j;
	for j = 0; j < 3; j = j + 1 {
		printf("j: %d\n", j);
	}

	ret 69;
}
