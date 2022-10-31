#include <stdio.h>
#include <time.h>

static time_t t;

static void fn_test_1(int a, int b)
{
	if ((a+b) > 8) printf("WTF??? %d,%d\n", a, b);
}

int main(void)
{
	int x = 0;
	t = time(NULL);
    fn_test_1(x, 2*(t%5));
	printf("we are good\n");
    return 0;
}

