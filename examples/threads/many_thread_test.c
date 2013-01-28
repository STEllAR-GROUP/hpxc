#include <hpxc/threads.h>
#include <stdio.h>
#include <assert.h>

hpxc_mutex_t io_lock;

void* creates_key(void* inputs){
    return NULL;
}

void* uses_key(void* inputs){
    int num=0;

    double some_work=1;
    int spin;
    for(spin=0;spin<1000000;spin++){
        some_work=some_work*1.1;
    }

    int result=num;
    assert(result==num);

    //hpxc_mutex_lock(&io_lock);
    printf("Expected %d, got %d\n",num,result);
    //hpxc_mutex_unlock(&io_lock);
    return NULL;
}

void my_init(){
    io_lock=HPXC_MUTEX_INITIALIZER;

    hpxc_thread_t threads[1000];

    hpxc_thread_t init;
    hpxc_thread_create(&init,NULL,creates_key,NULL);
    hpxc_thread_join(init,NULL);

    int i;
    for(i=0;i<1000;i++){
        hpxc_thread_create(&(threads[i]),NULL,uses_key,NULL);
    }
    for(i=0;i<1000;i++){
        hpxc_thread_join(threads[i],NULL);
    }
}

int main(int argc,char* argv[]){
    hpxc_init(my_init,argc,argv);
    return 0;
}
