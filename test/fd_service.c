#include "binder_common.h"
#include "binder_ipc.h"
#include "binder_io.h"
#include <sys/shm.h>

#define FD_SERVICE_NAME   "fd.service"


int fd_on_transact(uint32_t code, struct binder_io * recv, struct binder_io * reply, uint32_t flag){
    char * name = NULL;
    char *buf = 0;
    int fd = 0, i = 0;
    int len;
	int shm_id;

    if(!recv || !reply) return -1;

    name = binder_io_get_string(recv, NULL);

    printf("fd_on_transact get name from client : %s code: %d\n",name, code);

    fd = binder_io_get_fd(recv, 0);

    if(!fd) return -1;
    len = binder_io_get_uint32(recv);

	shm_id = binder_io_get_uint32(recv);
	buf = shmat(shm_id, NULL, 0);

    len = read(fd, buf, len);
/*
    printf("fd service read buf:\n");
    for(i = 0; i < len; i++)
        printf("0x%02x\t",buf[i]);
    printf("service close fd:%d\n",close(fd));
*/
	shmdt(buf);

    binder_io_append_uint32(reply, 0);


    return 0;
}

struct binder_service fd_service = {
    .on_transact = fd_on_transact,
    .link_to_death = NULL,
    .unlink_to_death = NULL,
    .death_notify = NULL,
};

int main(int argc, char * argv[]){
    binder_add_service(FD_SERVICE_NAME,&fd_service);

    binder_thread_enter_loop(1,1);

    binder_threads_shutdown();
}

