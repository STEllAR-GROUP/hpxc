#include <hpxc/pthread_compatibility.h>
#include <stdio.h>

void* print_hello(void* nothing){
    printf("hello!\n");
    pthread_exit(NULL);
    return NULL;
}

int hpxc_main(int argc, char** argv){
    pthread_t test;
    pthread_create(&test, NULL, print_hello, NULL);
    pthread_join(test,NULL);
    return 0;
}
