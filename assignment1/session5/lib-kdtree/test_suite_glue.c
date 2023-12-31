#include <stdio.h>

#include <uk/config.h>
#include <test_suite_glue.h>

int libkdtree_test_main() {

    int argc = 1;
    char **argv = NULL;
    
    int testCounter = 0;
    int errorCounter = 0;
    int rc;

// TODO_11
#if CONFIG_TEST_1
    testCounter++;
    printf("Running test y ....................\n");
    // TODO_11
    rc = test_y(argc, argv);
    if(rc == 0)
        printf("PASS\n");
    else {
        printf("FAIL\n");
        errorCounter++;
    }
#endif

// TODO_11
#if CONFIG_TEST_2
    testCounter++;
    printf("Running test z ....................\n");
    // TODO_11
    rc = test_z(argc, argv);
    if(rc == 0)
        printf("PASS\n");
    else {
        printf("FAIL\n");
        errorCounter++;
    }
#endif

    printf ("Total tests : %d\n", testCounter);
    printf ("Total errors: %d\n", errorCounter);

return errorCounter;
}
