
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

unsigned long               probs = 0;
unsigned long               correct = 0;
unsigned long long          time_start,time_end;
unsigned long long          time_unit;

struct prob {
    signed long long        answer;
    signed long long        a,b;
    char                    op;
    char                    correct;
};

/* MUST BE ODD */
#define PROBQUEUE           11

struct prob                 problist[PROBQUEUE] = { {0,0,0,0,0} };
int                         probgen=(PROBQUEUE/2);
const int                   probcursor=(PROBQUEUE/2);

void time_init(void) {
#if 1
    /* gettimeofday() */
    time_unit = 1000000;
#endif
}

void time_mark(unsigned long long *t) {
#if 1
    /* gettimeofday() */
    struct timeval tv;

    gettimeofday(&tv,NULL);

    *t  = (unsigned long long)tv.tv_sec * (unsigned long long)1000000;
    *t += (unsigned long long)tv.tv_usec;
#endif
}

/* take C-string and remove trailing newline. A C reimplementation of perl's chomp function */
/* for completeness, it is written to handle both Unix "LF" and DOS/Windows CR LF formats */
void chomp(char *s) {
	char *e = s + strlen(s) - 1;
	while (e >= s && (*e == '\n' || *e == '\r')) *e-- = 0;
}

const char oplist[4] = "+-*/";

void prob_adv(void) {
    unsigned int i;

    for (i=0;i < (PROBQUEUE-1);i++)
        problist[i] = problist[i+1];

    assert(i < PROBQUEUE);
    problist[i].op = 0;
    probgen--;
}

void prob_fill(void) {
    while (probgen < PROBQUEUE) {
        struct prob *p = &problist[probgen++];

again:
        p->op = oplist[((unsigned int)rand() % 4)];
        if (p->op == '/') {
            /* organize division always so that the result would be whole, keep it simple */
            p->b = 1 + ((unsigned int)rand() % 10);
            p->a = p->b * ((unsigned int)rand() % 10);

            p->answer = p->a / p->b;
        }
        else {
            p->b = ((unsigned int)rand() % 10);
            p->a = ((unsigned int)rand() % 10);

            /* for this version, let's not encourage negative answers */
            if (p->op == '-') {
                if (p->b > p->a) goto again;
                p->answer = p->a - p->b;
            }
            else if (p->op == '+') {
                p->answer = p->a + p->b;
            }
            else if (p->op == '*') {
                p->answer = p->a * p->b;
            }
        }
    }
}

/* WARNING: We assume DEC VT100 terminal codes are available */
void prob_draw(void) {
    unsigned int i;

    printf("\x1B" "[0m");   /* attributes off */
    printf("\x1B" "[2J");   /* erase screen */
    printf("\x1B" "[H");    /* home cursor */

    printf("Hit CTRL+D or type 'quit' to quit.\n");
    printf("\n");

    for (i=0;i < PROBQUEUE;i++) {
        struct prob *p = &problist[i];

        printf("\x1B" "[0m");   /* attributes off */
        if (p->op != 0) {
            if (i == probcursor)
                printf("\x1B" "[1;36m");    /* bright cyan */

            printf("%3lld %c %-3lld",
                (signed long long)p->a,
                p->op,
                (signed long long)p->b);

            if (p->correct > 0) {
                printf("\x1B" "[32m");      /* green */
                printf(" Y");
            }
            else if (p->correct < 0) {
                printf("\x1B" "[31m");      /* red */
                printf(" X  answer is %lld",(signed long long)p->answer);
            }
        }

        if (i == (PROBQUEUE/2))
            printf(" <--");

        printf("\n");
    }
    printf("\n");
    printf("ANSWER> ");
    fflush(stdout);
}

int main() {
    char line[512];

    srand(time(NULL));

    time_init();
    time_mark(&time_start);

    do {
        prob_fill();
        prob_draw();

        if (ferror(stdin))
            break;
        if (fgets(line,sizeof(line)-1,stdin) == NULL)
            break;
        chomp(line);

        if (!strcmp(line,"done") || !strcmp(line,"quit") || !strcmp(line,"\x04"/*CTRL+D*/))
            break;

        if (line[0] == '-' || isdigit(line[0])) {
            signed long long answer = strtoll(line,NULL,10);

            probs++;
            if (problist[probcursor].answer == answer) {
                problist[probcursor].correct = 1;
                correct++;
            }
            else {
                problist[probcursor].correct = -1;
            }

            prob_adv();
        }
    } while(1);

    time_mark(&time_end);

    printf("\n");
    printf("%lu problems answered in %.3f seconds\n",probs,((double)(time_end - time_start)) / 1000000);
    printf("%.3f problems/second\n",((double)probs * 1000000) / (time_end - time_start));
    printf("%lu correct out of %lu\n",correct,probs);
    return 0;
}

