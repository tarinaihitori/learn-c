#include <netinet/in.h>  // sockaddr_in構造体やINADDR_ANYなどIPv4通信関連の定義
#include <stdio.h>       // 標準入出力 (printf, perror など)
#include <stdlib.h>      // EXIT_FAILURE, EXIT_SUCCESS など
#include <string.h>      // memset, strlen など
#include <sys/socket.h>  // socket, bind, listen, accept, recv, send など
#include <sys/types.h>   // ソケットなどで使われる型の定義 (socklen_t など)
#include <unistd.h>      // close など

// netinet/in.h: IPv4 用のソケット通信に必要なデータ構造 (struct sockaddr_in) や定数 (INADDR_ANY) などが定義されています。
// sys/socket.h: ソケットの作成 (socket), ソケットに対する操作 (bind, listen, accept, recv, send) などが宣言されています。
// sys/types.h: socklen_t や pid_t などシステムで使われる型が定義されています。
// unistd.h: POSIX API (close など) が宣言されています。


#define PORT 8000

int main() {
    int sockfd;  // サーバ用のソケットディスクリプタです。socket() 関数から返ってくるファイルディスクリプタが格納されます。
    int new_sockfd;    // クライアントとの通信に使うソケットディスクリプタです。accept() 関数から返ってきます。
    socklen_t clilen;  // accept() 関数に渡されるクライアントアドレス構造体のサイズを示す変数です。
    char buffer[256];   // 受信データを一時的に格納するバッファです。
    struct sockaddr_in serv_addr, cli_addr;    // sockaddr_in は IPv4 アドレスを扱うための構造体です。サーバ用 (serv_addr) とクライアント用 (cli_addr) の 2 つを定義しています。
    int n;  // recv() や send() 関数の戻り値 (送受信バイト数) を受け取るために使用しています。


    // ソケットを作成する
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // domain: プロトコルファミリを指定します。AF_INET は IPv4 通信を表します。
    // type: ソケットの種類を指定します。SOCK_STREAM は TCP 通信を示します。
    // protocol: 通常は 0 を指定すると、上記 domain/type に合ったプロトコルが自動的に選択されます (今回は TCP)。
    // socket() は成功すると新しいソケットディスクリプタを返しますが、失敗すると -1 が返るので、エラー処理を行っています。
    // https://ja.manpages.org/socket/2



    // ソケットにアドレスを割り当てる
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                 // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;         // 任意のアドレス (0.0.0.0)
    serv_addr.sin_port = htons(PORT);               // ポート番号 (ネットワークバイトオーダー)
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }


    // memset()
    // serv_addr 構造体を 0 クリアしています。
    // serv_addr.sin_port = htons(PORT);
    // リトルエンディアン環境でも正しくポート番号を指定できるよう、htons() (host to network short) を使い、ネットワークバイトオーダーに変換しています。
    // bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))
    // ソケットにアドレス情報 (IPアドレス + ポート番号) を紐づけます。
    // 戻り値が 0 未満の場合はエラーです。



    // クライアントからの接続を待つ
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (new_sockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    // sockfd: 接続待ちにするソケットディスクリプタ
    // backlog: 接続要求がキューに入る最大数 (ここでは 5)。
    // 多くの OS ではこの値を超えても OS が調整するため、5 より多い接続が同時に来ても拒否はされずに調整されることが多いですが、基本は 5 以上にしておくのが無難です。
    // accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    // sockfd: listen() で接続待ちにしているソケットディスクリプタ
    // addr: 接続してきたクライアントのアドレス情報が格納されます (ここでは cli_addr)。
    // addrlen: cli_addr の構造体サイズを指定し、呼び出し後はクライアントアドレスの実際のサイズが書き込まれます。
    // この関数がブロッキングモードで呼び出される場合、クライアントからの接続があるまで処理が止まります。成功すると、クライアントとの通信に使う新しいソケットディスクリプタ (new_sockfd) を返します。


    // データを受信する
    memset(buffer, 0, 256);
    n = recv(new_sockfd, buffer, 255, 0);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Message from client: %s\n", buffer);

    // recv(int sockfd, void *buf, size_t len, int flags)
    // sockfd: 通信に使うソケットディスクリプタ (accept で得た new_sockfd)
    // buf: 受信データを格納するバッファ (ここでは buffer)
    // len: バッファのサイズ (255 文字分)
    // flags: 通常 0 (ブロッキング受信)
    // n は実際に受信したバイト数が返ります。エラー時は -1 となります。
    // printf("Message from client: %s\n", buffer);
    // 受信したメッセージを出力して確認しています。



    // データを送信する
    n = send(new_sockfd, "I got your message", 18, 0);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    // send(int sockfd, const void *buf, size_t len, int flags)
    // sockfd: 送信に使うソケットディスクリプタ
    // buf: 送信したいデータの先頭ポインタ
    // len: 送信するデータのバイト数
    // flags: 通常 0
    // 今回は "I got your message" (18 バイト分) を送信しています。成功すると、送信したバイト数が返ります。

    close(new_sockfd);
    close(sockfd);

    return 0;
}