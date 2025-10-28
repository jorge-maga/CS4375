#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char **argv)
{
    struct pstat uproc[NPROC];
    int nprocs;
    int i;
    char *state;
    static char *states[] = {
        [SLEEPING] "sleeping",
        [RUNNABLE] "runnable",
        [RUNNING] "running ",
        [ZOMBIE] "zombie  "};

    nprocs = getprocs(uproc);
    if (nprocs < 0)
        exit(-1);

    printf("pid\tstate\t\tsize\tppid\tpriority\tage\tname\n");

    for (i = 0; i < nprocs; i++) {
        state = states[uproc[i].state];
        if (state == 0) state = "unknown ";

        if (uproc[i].state == RUNNABLE) {
            int age = uptime() - (int)uproc[i].readytime;
            if (age < 0) age = 0; // safety
            printf("%d\t%s\t%d\t%d\t%d\t\t%d\t%s\n",
                uproc[i].pid,
                state,
                (int)uproc[i].size,
                uproc[i].ppid,
                uproc[i].priority,
                age,
                uproc[i].name);
        } else {
            printf("%d\t%s\t%d\t%d\t%d\t\tN/A\t%s\n",
                uproc[i].pid,
                state,
                (int)uproc[i].size,
                uproc[i].ppid,
                uproc[i].priority,
                uproc[i].name);
    }
}


    exit(0);
}