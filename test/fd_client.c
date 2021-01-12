#include "binder_common.h"
#include "binder_ipc.h"
#include "binder_io.h"
#include <sys/time.h>
#include <fcntl.h>

#define FD_SERVICE_NAME "fd.service"

unsigned int get_file_size(int fd)
{
    unsigned int length;

    length= lseek(fd,0L,SEEK_END);  
    lseek(fd,0L,SEEK_SET);  

	printf("length %d\n", length);
    return length;
}

int main(int argc, char * argv[]){
    struct timeval tpstart,tpend;
    long int v_sec = 0;
    struct binder_ipc_tinfo *ti = NULL;
    char binder_buf[DEFAULT_BINDER_IOBUF_SIZE] = {0};
    struct binder_io send, reply;
    char *buf;
    int fd = 0, len, i;

    if(argc < 2){
        printf("Usage : fd_client <filepath>\n");
        return -1;
    }
    uint32_t fd_handle = binder_get_service(FD_SERVICE_NAME);

    if(fd_handle == 0){
        printf("get fd handle fail...exit!\n");
        return -1;
    }

    fd = open(argv[1], O_RDONLY);
    if(fd <= 0) return -1;
    len = get_file_size(fd);
	buf = malloc(len);
    
    ti = binder_get_thread_info();
    binder_io_init(&send, binder_buf, sizeof(binder_buf), DEFAULT_OFFSET_LIST_SIZE);

    binder_io_append_string(&send, "fd_config_file");
    binder_io_append_fd(&send, fd);
    binder_io_append_uint32(&send,len);

    gettimeofday(&tpstart,NULL);
    memset(&reply,0,sizeof(reply));
    if(BINDER_STATUS_OK == binder_cmd_sync_call(ti,&send,&reply,fd_handle,0)){
        /*parse and free buffer*/
        binder_cmd_freebuf(ti, reply.data0);
    }
    gettimeofday(&tpend,NULL);
    v_sec = 1000000 *(tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);
    printf("binder sync call use time %ld(us)\n",v_sec);

    lseek(fd,0L,SEEK_SET);  //important
    len = read(fd, buf, len);
    printf("client read fd buf:\n");
    for(i = 0; i < len; i++)
            printf("0x%02x\t",buf[i]);
    printf("\n");

    printf("client close fd:%d\n",close(fd));
    
    binder_cmd_release(ti,fd_handle);
    flush_commands(ti);
    binder_threads_shutdown();    

    return 0;
}

