BROKER MQTT v.0.1

CONTEÚDO DESSE ARQUIVO
----------------------

 * Introdução
 * Estrutura
 * Configuração
 * Compilação e Execução
 * Funcionamento
 * Testes e Análise de Desempenho
 * Autor


INTRODUÇÃO
----------

O conteúdo desse repositório é a implementação de um Broker MQTT Versão 3.1.1, de acordo com as
especificações da OASIS. A estrutura, funcionamento e as instruções para compilação e execução do
broker e realização dos testes foram descritos abaixo.

Além da descrição deste LEIAME, o código está extensamente documentado no arquivos de cabeçalho,
caminho include/*. Também, todos os arquivos contêm um cabeçalho próprio em forma de comentário
explicando o conteúdo do arquivo.


ESTRUTURA
---------

De forma resumida, o repositório segue a seguinte estrutura:

ep1-henrique_carvalho
├── bin
│   └── broker 			- binário do broker
├── fonte
│   ├── broker.c 		- código do laço principal do broker
│   ├── handlers.c 		- código com os manejadores de mensagens recebidas
│   ├── log.c 			- código com funções de logging
│   ├── session.c 		- código com funções que administram uma sessão entre broker cliente
│   ├── utils.c			- código com funções auxiliares do projeto
│   └── wrappers.c		- código com wrappers para algumas funções
│
├── include			- headers com definições dos arquivos .c
│   ├── config.h		- define configurações do broker
│   ├── rc.h			- define tipos de código de retorno que controlam o fluxo entre funções
│   ├── handlers.h
│   ├── log.h
│   ├── internal.h
│   ├── utils.h
│   └── wrappers.h
│
├── LEIAME
├── Makefile
├── slides.pdf
└── testes
    ├── bin			- binários de testes unitários
    ├── scripts
    │   ├── clients.sh		- script para geração de clientes
    │   └── collect.sh		- script para coletar dados de CPU e Rede
    └── testes_unitários	- arquivos .c de testes unitários


CONFIGURAÇÃO
------------

Algumas configurações de log podem ser modificadas antes da execução do broker. As configurações que
podem ser editadas diretamente no arquivo include/config.h:

	LOGGING      # 1 para habilitar mensagens de log e 0 para desabilitar (desabilita inclusive
		       mensagens de erro)
	TERMCOLORS   # 1 para habilitar cores no terminal e 0 para desabilitar

Por padrão essas configurações estão habilitadas.


COMPILAÇÃO E EXECUÇÃO
---------------------

Para compilar, basta acessar o diretório ep1-henrique_carvalho e digitar `make` no shell. O binário
broker será criado no diretório bin/. A execução do broker é feita digitando o programa e a porta em que
irá funcionar o programa:

	./bin/broker <Porta>

Exemplo de execução:

	user@host $ ./bin/broker 8888
	[12:17:03.083944] Iniciando MQTT Broker v.3.1.1. Pressione Ctrl-C para terminar.
	[12:17:03.084098] Criado diretório de tópicos em : /tmp/mqtt-topics-1650899823/.
	[12:17:03.084142] Escutando na porta 8888.

Quanto as mensagens de log acima aparecerem, o broker estará pronto para receber conexões.


FUNCIONAMENTO
-------------

Toda conexão com o broker, cria um processo responsável pelo cliente e esse processo entra no loop
de manejo de mensagens. Uma mensagem recebida é decodificada em um pacote e, então, essa mensagem é
manejada de alguma forma específica, de acordo com as especificações do MQTT versão 3.1.1. Este
broker suporta mensagens de CONNECT, DISCONNECT, PINGREQ, SUBSCRIBE e PUBLISH.

O sistema de manejo de SUBSCRIBE e PUBLISH funciona, basicamente, por meio da escrita e leitura de
um arquivo. Um tópico criado por um processo que maneja um PUBLISH é um arquivo numa pasta
temporária configurada em include/config.h, na macro TMP_FILEPATH. Uma publicação sobrescreve o
arquivo de mesmo tópico e uma inscrição lê desse arquivo. Por exemplo, se um cliente se inscreve
no tópico 'temperatura', o broker cria um processo para ler no tópico
/tmp/mqtt-topics-XXXXXXXXXX/temperatura e outro para acompanhar novas mensagens de PING do cliente
(os 'X' do caminho do tópico são o tempo em segundos em que o arquivo foi criado). Se um cliente
publica em 'temperatura', o broker cria um processo para escrever neste tópico.

Por padrão, o broker adota o mesmo comportamento do mosquitto quando uma mensagem é publicada em um
tópico antes de uma inscrição, essa mensagem não é enviada ao cliente inscrito no tópico.

O broker não implementa algumas flags de CONNECT e, portanto, não aceita várias opções normalmente
aceitas pelo mosquitto. Em suma, o broker recebe conexões dos clientes mosquitto_sub e mosquitto_pub
com as seguintes opções:

	mosquitto_sub -h <endereço de ip> -p <porta> -t <tópico> -V <versão: 311>
	mosquitto_pub -h <endereço de ip> -p <porta> -t <tópico> -m <mensagem> -V <versão: 311>

Note que a <versão> suportada é apenas a 3.1.1 e, portanto, se a -V for passada, deve ser com
`-V 311`.

O broker não implementa wildcards e, portanto, a tentativa de inscrição ou publicação em tópicos
usando qualquer wildcard contida na seção 4.7 da especificação tem comportamento indefinido.


TESTES E ANÁLISE DE DESEMPENHO
------------------------------

Alguns testes unitários foram criados para testar funções específicas e o comportamento específico
de algumas partes do código. Estes testes se encontram em testes/testes_unitarios, mas eles não estão
documentados, sua compilação não é garantida e sua execução não é recomendada.

Testes para análise de desempenho são os testes de clientes simultâneos. Esses podem ser realizados
com os scripts clients.sh e collect.sh, em testes/scripts. O script de nome cpu.sh deve ser executado
antes, com o comando `./collect.sh <porta> <prefixo do arquivo rastro>` e clients.sh deve ser executado
no computador que irá rodar os clientes com o comando
`./clients.sh <número de clientes> <endereço de ip> <porta>`. O arquivo de dados retorna um formato
de "rastros" de `ps ux` por tempo, delimitados por caracteres "+" e ficará salvo em analise/dados.
Um outro arquivo de dados é criado em analise/dados com a análise do consumo de rede, gerado a
partir do programa nethogs.

Em analise/notebooks estão os códigos, no formato de jupyter notebook, da limpeza dos arquivos de rastros
e das análises realizadas.


LICENÇA
-------

Licenciado sob GNU Public License version 3 (GPLv3).


AUTOR
-----

Henrique de Carvalho <henriquecarvalho@usp.br>
