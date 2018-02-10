/* malloc.c */

void *malloc(unsigned long n);

int main(void)
{
    int n;
    int *p;

    n = 10;
    p = malloc(sizeof *p * n);

    while (-- n)
	*p ++ = 0;
}
