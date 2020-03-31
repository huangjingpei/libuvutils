#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_loop_t *loop;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {//用于读数据时的缓冲区分配
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);//这是个构造函数，这里仅仅是浅拷贝！指针只是进行了赋值而已！！！
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(loop, client, 0);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {//accept成功之后开始读
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);//开始从pipe读（在loop中注册读事件），读完回调
    }
    else {
        uv_close((uv_handle_t*) client, NULL);//出错关闭
    }
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, "echo.sock", NULL);//删除操作
    exit(0);
}

int main() {
    loop = uv_default_loop();

    uv_pipe_t server;
    uv_pipe_init(loop, &server, 0);

    signal(SIGINT, remove_sock);//收到结束信号顺便删除相关pipe文件

    int r;
    if ((r = uv_pipe_bind(&server, "echo.sock"))) {//服务器端绑定unix域地址
        fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return 1;
    }
    if ((r = uv_listen((uv_stream_t*) &server, 128, on_new_connection))) {//开始连接，出现回调，enent loop由此开始
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 2;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
