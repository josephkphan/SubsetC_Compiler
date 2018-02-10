/* tricky.c */

/***
 *** a tricky comment ******/

int printf(char *s, ...);

int main(void)
{
    auto int a, b, c;
    unsigned long long_int;

    a = 0;
    b = 123;
    c = 0x123aF;

    printf("I said \"hello there\"");
    printf("I said %chello there%c", '\'', '\'');
}
