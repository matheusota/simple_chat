Serviço de Bate-Papo - MC833

Desenvolvido por:
Gustavo de Mello Crivelli - RA 136008
Matheus Jun Ota           - RA 138889


Conteúdo
--------

servidor.c - Código fonte do servidor UDP concorrente.
cliente.c  - Código fonte do cliente de chat.
Makefile   - Usado para compilação dos programas.


Compilação
----------

Extraia os arquivos para uma pasta. Pelo terminal, acesse o diretório e execute:

$ make


Execução
--------

Após a compilação, o servidor pode ser executado pelo terminal passando por parâmetro a porta pública a ser utilizada:

./servidor [porta_servidor]


Para executar o cliente, use o comando:

./cliente [IP_servidor | nome_servidor] [porta_servidor]

Portanto, é preciso passar como argumento o IP ou nome do servidor, e a porta pública do mesmo.


Funcionalidades
---------------

Os seguintes comandos estão disponiveis para o cliente do chat:

\list                     - Lista todos os usuários conectados.
\name [nome_do_usuario]   - Registra o nickname do cliente no servidor.
\friend [nome_do_usuario] - Inicia uma conversa com o usuário especificado.
\file [nome_do_arquivo]   - Transfere arquivo especificado para o cliente na outra ponta da conversa.
\exit                     - Encerra conexão do cliente com o servidor.

Ao realizar uma transferência de arquivo, este será salvo na outra ponta, no mesmo diretório onde se encontra o executável do cliente, com o nome "output.txt".


Registro de Atividades
----------------------

O servidor registra todas as operações de conexão, desconexão, conversas e outras operações dos clientes em um arquivo nomeado "log.txt", que será gerado no mesmo diretório do servidor.



