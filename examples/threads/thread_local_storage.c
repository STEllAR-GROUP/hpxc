#include <hpxc/threads.h>
#include <stdio.h>
#include <assert.h>

hpxc_key_t key;
hpxc_mutex_t counter_lock;
int counter;

hpxc_mutex_t io_lock;

void* creates_key(void* inputs){
    //int ret=hpxc_key_create(&key,NULL);
    return NULL;
}

void* uses_key(void* inputs){
    //assert(hpxc_getspecific(key)==NULL);
    hpxc_mutex_lock(&counter_lock);
    int num=counter++;
    hpxc_mutex_unlock(&counter_lock);
    //hpxc_setspecific(key,(void*)num);

    int some_work=0;
    int spin;
    for(spin=0;spin<1000000;spin++){
        some_work++;
    }

    //int result=(int)hpxc_getspecific(key);
    int result=num; //to make up for previous line
    assert(result==num);

    hpxc_mutex_lock(&io_lock);
    printf("Expected %d, got %d\n",num,result);
    hpxc_mutex_unlock(&io_lock);
    return NULL;
}

int main(){
    counter=0;
    counter_lock=HPXC_MUTEX_INITIALIZER;
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
    return 0;
}
