int rand(void);

int main(void)
{
    int i, x;
    int a[10];

    for (i = 0; i < 10; i = i + 1)
	a[i] = rand() % 10;

    x = rand() % 10;

    for (i = 0; i < 10; i = i + 1)
	if (x == a[i])
	    break;

    return i < 10;
}
