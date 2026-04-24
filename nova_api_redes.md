_send_(socket, buffer, size) -> envia a mensagem inteira
_confirmSend_(socket) -> envia fim de mensagem em um loop até receber ACK

_receive_(socket, buffer, size) -> recebe uma mensagem e guarda em um buffer
_confirmReceive_(socket) -> envia início de mensagem em loop até receber ACK

Todas as funções são "bloqueantes".

```c
// futuramente, essa função ainda vai quebrar o buffer em pacotes caso precise
void send(int socket, void* buffer, int size) {
    // cria packets do tipo .data
    mensagem = cria_mensagem(DATA, buffer, size);

    while (true) {
        sucesso = mensagem.envia_mensagem(socket);
        if (!sucesso) {
            continue;
        }

        mensagem resposta;
        bool msg_valida;
        while (true) {
            resultado = resposta.receiveMessage(socket);

            if (resultado == timeout) {
                break;
            } else if (resultado == mensagem_inválida) {
                msg_valida = false;
                continue;
            } else {
                msg_valida = true;
                break;
            }
        }

        if (msg_valida) {
            if (resposta.tipo == ack) {
                return;
            } else {
                continue;
            }
        }
    }
}
```

```c
void confirmSend(socket) {
    mensagem = criaMensagem(MESSAGE_END, NULL, 0);
    do {
        resultado = sendMessage(socket, mensagem);
        if (resultado != sucesso) {
            continue;
        }

        resposta = receiveMessage(socket);

        if (resposta == timeout || resposta = mensagem_invalida) {
            msg_valida = false;
        } else {
            msg_valida = true;
        }

    } while (msg_valida = false);
}
```

```c
// - é esperado que quem utilize essa api saiba de antemão o tamanho máximo das
// mensagens transmitidas entre servidor e cliente;
// - caso seja enviada uma mensagem maior do que buffer_size, os bytes
//sobressalentes serão descartados
void receive(int socket, void* buffer, unsigned int buffer_size) {
    unsigned int tamanho_mensagem_atual = 0;
    mensagem mensagem;

    // TODO: provavelmente precisa de um loop pra receber mensagem do tipo MESSAGE_INIT
    while (true) {
        // lógica para receber um message init
        // se receber uma mensagem normal, passar para o próximo
        // loop e processar mensagem normalmente
    }

    while(true) {
        // TODO: estruturar a lógica de loops para receber mensagens
        // (é pra ser parecida com a do loop que já tá no cliente)
        mensagem.receiveMessage();

        // ultrapassamos o limite do buffer oferecido
        if (tamanho_mensagem_atual + mensagem.header.size > buffer_size) return;

        memcpy(buffer, mensagem.data, mensagem.header.size);
        tamanho_mensagem_atual += mensagem.header.size;
    }
}
```

```c
void confirmReceive(int socket) {
    mensagem = criaMensagem(MESSAGE_INIT, NULL, 0);
    do {
        resultado = mensagem.sendMessage();
        if (resultado != sucesso) {
            continue;
        }

        resposta = receiveMessage(socket);

        if (resposta == timeout || resposta == mensagem_invalida) {
            msg_valida = false;
        } else {
            if (resposta.type != ack) continue;
            msg_valida = true;
        }

    } while (msg_valida = false);
}
```
