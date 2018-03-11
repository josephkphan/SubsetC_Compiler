/* Oh, for the lack of character constants */

int NUM;
int lookahead, expr(void);

int c, lexval;
int getchar(void), isdigit(int c), isspace(int c), printf(char *s, ...);
void exit(int n);

int lexan(void)
{
    int n;


    if (c == 0)
	c = getchar();

    while (isspace(c) && c != '\n')
	c = getchar();

    if (!isdigit(c)) {
	int x;
	x = c;
	c = 0;
	return x;
    }

    n = 0;

    while (isdigit(c)) {
	n = n * 10 + c - 48;
	c = getchar();
    }

    lexval = n;
    return NUM;
}


/* Look familiar? */

void match(int token)
{
    if (lookahead != token) {
	printf("syntax error at %d\n", lookahead);
	exit(1);
    }

    lookahead = lexan();
}


int factor(void)
{
    int n;


    if (lookahead == '(') {
	match('(');
	n = expr();
	match(')');
	return n;
    }

    n = lexval;
    match(NUM);
    return n;
}


int term(void)
{
    int n;
    
    
    n = factor();

    while (1) {
	if (lookahead == '*') {
	    match('*');
	    n = n * factor();

	} else if (lookahead == '/') {
	    match('/');
	    n = n / factor();

	} else
	    break;
    }

    return n;
}


int expr(void)
{
    int n;


    n = term();

    while (1) {
	if (lookahead == '+') {
	    match('+');
	    n = n + term();

	} else if (lookahead == '-') {
	    match('-');
	    n = n - term();

	} else
	    break;
    }

    return n;
}


int main(void)
{
    int n;


    lookahead = lexan();

    while (lookahead != -1) {
	n = expr();
	printf("%d\n", n);

	while (lookahead == '\n')
	    match('\n');
    }
}
