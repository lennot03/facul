#include <stdio.h>
#include "raylib.h"
#include <time.h>
#include <math.h>

#define ALTURA 600
#define LARGURA 1200

#define LINHAS 30
#define COLUNAS 60

#define TAM_BLOCO 20

#define MAX_INIMIGOS 50

//declaração de structs
typedef struct
{
    int col;
    int lin;
    int vidas;
    int recursos;
    int nivel;
} JOGADOR;  //struct para o jogador

typedef struct
{
    int col;
    int lin;
    int ativo;
    int ultimoMovimento; // 0: direita, 1: esquerda, 2: baixo, 3: cima
} INIMIGO;  //struct para os inimigos

typedef struct
{
    int jogador;
    char tipo;
} QUADRADO;  //struct para desenhar o mapa

typedef struct
{
    int col;
    int lin;
    int vidas;
} BASE;  //struct para a base

//função que carrega o arquivo do mapa
int carregaMapa(char *fileName, QUADRADO mapa[LINHAS][COLUNAS], JOGADOR *jogador, INIMIGO inimigos[MAX_INIMIGOS], BASE *base,  int *devePularMenu, int *estadoJogo, int *opcao_menu, int *jogoEmAndamento)
{
    int i, j;
    char linha[COLUNAS];
    QUADRADO quadrado;

    FILE *mapaFile = fopen(fileName, "r");
    if (mapaFile == NULL)
    {
        *estadoJogo = 1;
        *jogoEmAndamento = 0;
    }
    else
    {
        for(i = 0; i < MAX_INIMIGOS; i++)
        {
            inimigos[i].ativo = 0;
        }
        for (i = 0; i < LINHAS; i++)
        {
            fscanf(mapaFile, "%[^\n]%*c", linha);
            for (j = 0; j < COLUNAS; j++)
            {
                char tipo = linha[j];
                if (tipo == 'J')  //guarda informações da posição do jogador dentro do arquivo
                {
                    quadrado.jogador = 1;
                    quadrado.tipo = ' ';
                    jogador->col = j;
                    jogador->lin = i;
                }
                else if (tipo == 'M')  //adiciona inimigos na função
                {
                    adicionaInimigo(inimigos, j, i);
                }
                else
                {
                    quadrado.tipo = tipo;  //guarda informações do que é cada caractere
                    quadrado.jogador = 0;
                    if(tipo == 'S')  //se for a base, guarda a posição dela (para o cálculo de movimentação dos inimigos
                    {
                        base->col = j;
                        base->lin = i;
                    }
                }
                mapa[i][j] = quadrado;
            }
        }
    }

    fclose(mapaFile);
    return 1;
}

//desenha o mapa do jogo, transformando os caracteres em quadrados 20x20
void desenhaMapa(QUADRADO mapa[LINHAS][COLUNAS])
{
    int i, j;
    QUADRADO quadrado;

    for (i = 0; i < LINHAS; i++)
    {
        for (j = 0; j < COLUNAS; j++)
        {
            QUADRADO quadrado = mapa[i][j];
            switch (quadrado.tipo)
            {
            case 'R':  //caso recurso
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, DARKGREEN);
                break;
            case 'H':  //caso buraco
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, BLUE);
                break;
            case 'S':  //caso base
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, YELLOW);
                break;
            case 'W':  //baso parede
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, BROWN);
                break;
            case 'O':  //caso obstáculo (recurso colocado pelo jogador)
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, PURPLE);
                break;
            case ' ':  //caso em que não tem nenhum caractere
                DrawRectangle(j * TAM_BLOCO, i * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, BLACK);
                break;
            }
        }
    }
}

//função específica para o desenho do jogador
void desenhaJogador(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS])
{
    int col = jogador->col;
    int lin = jogador->lin;

    DrawRectangle(col * TAM_BLOCO, lin * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, PINK);
}

//função específica para o desenho dos inimigos
void desenhaInimigo(INIMIGO inimigos[MAX_INIMIGOS])
{
    int i;
    for (i = 0; i < MAX_INIMIGOS; i++)
    {
        if (inimigos[i].ativo)
        {
            DrawRectangle(inimigos[i].col * TAM_BLOCO, inimigos[i].lin * TAM_BLOCO, TAM_BLOCO, TAM_BLOCO, RED);
        }
    }
}

//essa função adiciona os inimigos e, para cada inimigo[i], coloca que inimigo.ativo = 1 para mostrar que ele está vivo
int adicionaInimigo(INIMIGO inimigos[MAX_INIMIGOS], int col, int lin)
{
    int i;
    for (i = 0; i < MAX_INIMIGOS; i++)
    {
        if (!inimigos[i].ativo)
        {
            inimigos[i].col = col;
            inimigos[i].lin = lin;
            inimigos[i].ativo = 1;
            return 1;
        }
    }
    return 0;
}

//função que incializa as informações do jogador
void iniciarJogador(JOGADOR *jogador)
{
    jogador->vidas = 1;
    jogador->recursos = 3;
    jogador->nivel = 1;
}

//função de movimentação do jogador
void moveJogador(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS], INIMIGO inimigos[MAX_INIMIGOS]) {
    int col = jogador->col;
    int lin = jogador->lin;
    int novaCol = col;
    int novaLin = lin;
    int direcao = -1;

    mapa[lin][col].jogador = 0;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {  //caso em que o jogador anda para a direita, podendo andar por espaços vazios, buracos, recursos, monstros e obstáculos
        if (mapa[lin][col + 1].tipo == ' ' || mapa[lin][col + 1].tipo == 'H' || mapa[lin][col + 1].tipo == 'R' || mapa[lin][col + 1].tipo == 'M' || mapa[lin][col + 1].tipo == 'O') {
            novaCol++;
            direcao = 0;  //direção = 0 indica direita
        }
    } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {  //caso em que o jogador anda para a esquerda, podendo andar por espaços vazios, buracos, recursos, monstros e obstáculos
        if (mapa[lin][col - 1].tipo == ' ' || mapa[lin][col - 1].tipo == 'H' || mapa[lin][col - 1].tipo == 'R' || mapa[lin][col - 1].tipo == 'M' || mapa[lin][col - 1].tipo == 'O') {
            novaCol--;
            direcao = 1;  //direção = 1 indica esquerda
        }
    } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {  //caso em que o jogador anda para baixo, podendo andar por espaços vazios, buracos, recursos, monstros e obstáculos
        if (mapa[lin + 1][col].tipo == ' ' || mapa[lin + 1][col].tipo == 'H' || mapa[lin + 1][col].tipo == 'R' || mapa[lin + 1][col].tipo == 'M' || mapa[lin + 1][col].tipo == 'O') {
            novaLin++;
            direcao = 2;  //direção = 2 indica para baixo
        }
    } else if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {  //caso em que o jogador anda para cima, podendo andar por espaços vazios, buracos, recursos, monstros e obstáculos
        if (mapa[lin - 1][col].tipo == ' ' || mapa[lin - 1][col].tipo == 'H' || mapa[lin - 1][col].tipo == 'R' || mapa[lin - 1][col].tipo == 'M' || mapa[lin - 1][col].tipo == 'O') {
            novaLin--;
            direcao = 3;  //direçãp = 3 indica para cima
        }
    }

    if (mapa[novaLin][novaCol].tipo == 'H') {  //se o jogador passar por um buraco, faz o chamamento da função de teletransporte
        teletransportaJogador(jogador, mapa, direcao);
    }
    else if(mapa[novaLin][novaCol].tipo == 'R')  //se for um recurso, adiciona um recuso nas informações do jogador
    {
        if((jogador->recursos + 1) % 2 == 0)  //a cada dois recursos que o jogador pegar, sua vida é incrementada em 1
        {
            jogador->vidas++;
        }
        jogador->recursos++;
        mapa[novaLin][novaCol].tipo = ' ';  //após adicionar um recurso, coloca que aquele espaço do mapa agora é vazio
    }
    else
    {
        jogador->col = novaCol;  //se não for nem recurso, nem buraco, guarda as informações de movimentação do jogador
        jogador->lin = novaLin;
    }
    mapa[jogador->lin][jogador->col].jogador = 1;
}

//função para teletransporte do jogador por buracos
void teletransportaJogador(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS], int direcao)
{
    int col = jogador->col;
    int lin = jogador->lin;
    int novaCol = col;
    int novaLin = lin;
    int i;

    if (direcao == 0) //caso em que ele anda para a direita
    {
        for (i = 0; i < COLUNAS; i++)
        {
            if (mapa[lin][i].tipo == 'H')
            {
                novaCol = i;  //analisa o mapa da esquerda para a direita, sendo i sua nova posição
            }
        }
    }
    else if (direcao == 1) //caso em que ele anda para a esquerda
    {
        for (i = COLUNAS - 1; i >= 0; i--)
        {
            if (mapa[lin][i].tipo == 'H')
            {
                novaCol = i;  //analisa o mapa da direita para a esquerda, sendo i sua nova posição
            }
        }
    }
    else if (direcao == 2) //caso em que ele anda para baixo
    {
        for (i = 0; i < LINHAS; i++)
        {
            if (mapa[i][col].tipo == 'H')
            {
                novaLin = i;  //analisa o mapa de cima para baixo, sendo i sua nova posição
            }
        }
    }
    else if (direcao == 3) //caso em que ele anda para cima
    {
        for (i = LINHAS - 1; i >= 0; i--)
        {
            if (mapa[i][col].tipo == 'H')
            {
                novaLin = i;  //analisa o mapa de baixo para cima, sendo i sua nova posição
            }
        }
    }

    //guarda as informações da nova posição do jogador
    jogador->col = novaCol;
    jogador->lin = novaLin;
}

//função específica para colocar os recursos do jogador dentro do mapa
void colocaRecurso(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS])
{
    if(IsKeyPressed(KEY_G) && jogador->recursos > 0 && mapa[jogador->lin][jogador->col].tipo == ' ')  //analisa se o jogador TEM recursos e se o espaço do mapa é vazio
    {
        mapa[jogador->lin][jogador->col].tipo = 'O';
        jogador->recursos--;  //diminui uma unidade de recurso do jogador
    }
}

//função para a movimentação do inimigo
void moveInimigo(INIMIGO inimigos[MAX_INIMIGOS], QUADRADO mapa[LINHAS][COLUNAS], BASE *base, JOGADOR *jogador)
{
    int baseCol = base->col;
    int baseLin = base->lin;
    int i;

    for (i = 0; i < MAX_INIMIGOS; i++)
    {
        if (inimigos[i].ativo)  //verifica se o inimigo[i] está ativo
        {
            int melhorDist = 9999;  //variável para guardar a distância até a base
            int novaCol = inimigos[i].col;
            int novaLin = inimigos[i].lin;
            int direcao = 4;

            //verifica se é possível ir para a direita
            if (inimigos[i].col + 1 < COLUNAS && (mapa[inimigos[i].lin][inimigos[i].col + 1].tipo == ' ' || mapa[inimigos[i].lin][inimigos[i].col + 1].tipo == 'R' || mapa[inimigos[i].lin][inimigos[i].col + 1].tipo == 'O' || mapa[inimigos[i].lin][inimigos[i].col + 1].tipo == 'S') && inimigos[i].ultimoMovimento != 1)
            {
                int dist = abs((inimigos[i].col + 1) - baseCol) + abs(inimigos[i].lin - baseLin);
                if (dist < melhorDist)
                {
                    melhorDist = dist;  //atualiza a melhor distância
                    direcao = 0;
                }
            }

            //verifica se é possível ir para a esquerda
            if (inimigos[i].col - 1 >= 0 && (mapa[inimigos[i].lin][inimigos[i].col - 1].tipo == ' ' || mapa[inimigos[i].lin][inimigos[i].col - 1].tipo == 'R' || mapa[inimigos[i].lin][inimigos[i].col - 1].tipo == 'O' || mapa[inimigos[i].lin][inimigos[i].col - 1].tipo == 'S') && inimigos[i].ultimoMovimento != 0)
            {
                int dist = abs((inimigos[i].col - 1) - baseCol) + abs(inimigos[i].lin - baseLin);
                if (dist < melhorDist)
                {
                    melhorDist = dist;  //atualiza a melhor distância
                    direcao = 1;
                }
            }

            //verifica se é possível ir para baixo
            if (inimigos[i].lin + 1 < LINHAS && (mapa[inimigos[i].lin + 1][inimigos[i].col].tipo == ' ' || mapa[inimigos[i].lin + 1][inimigos[i].col].tipo == 'R' || mapa[inimigos[i].lin + 1][inimigos[i].col].tipo == 'O' || mapa[inimigos[i].lin + 1][inimigos[i].col].tipo == 'S') && inimigos[i].ultimoMovimento != 3)
            {
                int dist = abs(inimigos[i].col - baseCol) + abs((inimigos[i].lin + 1) - baseLin);
                if (dist < melhorDist)
                {
                    melhorDist = dist;  //atualiza a melhor distância
                    direcao = 2;
                }
            }

            //verifica se é possível ir para cima
            if (inimigos[i].lin - 1 >= 0 && (mapa[inimigos[i].lin - 1][inimigos[i].col].tipo == ' ' || mapa[inimigos[i].lin - 1][inimigos[i].col].tipo == 'R' || mapa[inimigos[i].lin - 1][inimigos[i].col].tipo == 'O' || mapa[inimigos[i].lin - 1][inimigos[i].col].tipo == 'S') && inimigos[i].ultimoMovimento != 2)
            {
                int dist = abs(inimigos[i].col - baseCol) + abs((inimigos[i].lin - 1) - baseLin);
                if (dist < melhorDist)
                {
                    melhorDist = dist;  //atualiza a melhor distância
                    direcao = 3;
                }
            }

            //atualiza a posição do inimigo baseado na melhor direção
            switch(direcao)
            {
                case 0:  //direita
                    novaCol = inimigos[i].col + 1;
                    novaLin = inimigos[i].lin;
                    break;
                case 1:  //esquerda
                    novaCol = inimigos[i].col - 1;
                    novaLin = inimigos[i].lin;
                    break;
                case 2:  //baixo
                    novaCol = inimigos[i].col;
                    novaLin = inimigos[i].lin + 1;
                    break;
                case 3:  //cima
                    novaCol = inimigos[i].col;
                    novaLin = inimigos[i].lin - 1;
                    break;
            }

            //atualiza a posição do inimigo
            inimigos[i].col = novaCol;
            inimigos[i].lin = novaLin;
            inimigos[i].ultimoMovimento = direcao;

            if(jogador->col == novaCol && jogador->lin == novaLin)  //se a posição do inimigo for igual a posição do jogador
            {
                jogador->vidas--;  //vida do jogador é diminuída em 1
                inimigos[i].ativo = 0;  //inimigo fica inativo
            }
            else if(mapa[novaLin][novaCol].tipo == 'S')  //se a posição do inimigo for igual a posição da base
            {
                base->vidas--;  //vida da base é diminuída em 1
                inimigos[i].ativo = 0;  //inimigo fica inativo
            }
            else if(mapa[novaLin][novaCol].tipo == 'O')  //se a posição do inimigo for igual a posição de um obstáculo
            {
                mapa[novaLin][novaCol].tipo = ' ';  //espaço fica em branco
                inimigos[i].ativo = 0;  //inimigo fica inativo
                base->vidas++;  //sempre que o jogador matar um inimigo, a base ganha 1 vida
            }
        }
    }
}

//função para pausar o jogo
int pausarJogo()
{
    if(IsKeyPressed(KEY_TAB))
    {
        return 1;
    }
}

//função para salvar o jogo
void salvarJogo(char *nomeArquivo, JOGADOR jogador, INIMIGO inimigos[MAX_INIMIGOS], BASE base, QUADRADO mapa[LINHAS][COLUNAS]) {
    FILE *arquivo = fopen(nomeArquivo, "wb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo para salvar o jogo.\n");
        return;
    }

    fwrite(&jogador, sizeof(JOGADOR), 1, arquivo);
    fwrite(inimigos, sizeof(INIMIGO), MAX_INIMIGOS, arquivo);
    fwrite(&base, sizeof(BASE), 1, arquivo);
    fwrite(mapa, sizeof(QUADRADO), LINHAS * COLUNAS, arquivo);

    fclose(arquivo);
    printf("Jogo salvo com sucesso em %s.\n", nomeArquivo);
}

//função para carregar o jogo salvo
void carregarJogoSalvo(char *nomeArquivo, JOGADOR *jogador, INIMIGO inimigos[MAX_INIMIGOS], BASE *base, QUADRADO mapa[LINHAS][COLUNAS]) {
    FILE *arquivo = fopen(nomeArquivo, "rb");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo para carregar o jogo.\n");
        return;
    }

    fread(jogador, sizeof(JOGADOR), 1, arquivo);
    fread(inimigos, sizeof(INIMIGO), MAX_INIMIGOS, arquivo);
    fread(base, sizeof(BASE), 1, arquivo);
    fread(mapa, sizeof(QUADRADO), LINHAS * COLUNAS, arquivo);

    fclose(arquivo);
    printf("Jogo carregado com sucesso de %s.\n", nomeArquivo);
}

//função que desenha o menu inicial do jogo
int desenhaMenu()
{
    BeginDrawing();
    Texture2D menu_capa = LoadTexture("CAPA.png");
    DrawTexturePro(menu_capa, (Rectangle){0, 0, LARGURA, ALTURA}, (Rectangle){0, 0, LARGURA, ALTURA}, (Vector2){0, 0}, 0, WHITE);
    DrawText("TOWER DEFENSE", (LARGURA / 2) - 420, 200, 90, RED);
    DrawText("NOVO JOGO [N]", (LARGURA / 2) - 130, 350, 30, RED);
    DrawText("CARREGAR JOGO [C]", (LARGURA / 2) - 175, 425, 30, RED);
    DrawText("SAIR [Q]", (LARGURA / 2) - 80, 500, 30, RED);
    EndDrawing();

    if (IsKeyPressed(KEY_N))  //caso em que um novo jogo é criado
    {
        return 1;
    }
    else if(IsKeyPressed(KEY_C))  //caso em que um jogo é carregado
    {
        return 2;
    }
    else if (IsKeyPressed(KEY_Q))  //caso em que o jogador somente sai
    {
        return 3;
    }
    return 0;
}

//função que desenha o menu de pause
int desenhaMenuPause()
{
    int opcao = 0;

    while (opcao == 0)
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("PAUSE MENU", ((LARGURA / 2) - 150), 200, 40, DARKGRAY);
        DrawText("C - Continuar", ((LARGURA / 2) - 150), 300, 30, GRAY);
        DrawText("L - Carregar jogo", ((LARGURA / 2) - 150), 350, 30, GRAY);
        DrawText("S - Salvar jogo", ((LARGURA / 2) - 150), 400, 30, GRAY);
        DrawText("V - Voltar ao menu", ((LARGURA / 2) - 150), 450, 30, GRAY);
        DrawText("F - Sair", ((LARGURA / 2) - 150), 500, 30, GRAY);

        EndDrawing();

        if (IsKeyPressed(KEY_C))
        {
            opcao = 1; //continuar o jogo
        }
        else if (IsKeyPressed(KEY_L))
        {
            opcao = 2; //carregar jogo salvo
        }
        else if (IsKeyPressed(KEY_S))
        {
            opcao = 3; //salvar o jogo atual
        }
        else if (IsKeyPressed(KEY_V))
        {
            opcao = 4; //voltar ao menu inicial
        }
        else if (IsKeyPressed(KEY_F))
        {
            opcao = 5; //sair do jogo sem salvar
        }
    }

    return opcao;
}

//função que desenha na tela as informações do jogador, da base e dos inimigos
void desenhaInfo(int nivel, int vidasJogador, int recursos, int vidasBase, int inimigosVivos) {
    char texto[50];

    sprintf(texto, "Nivel: %d", nivel);
    DrawText(texto, 20, 15, 22, RED);

    sprintf(texto, "Vidas do Jogador: %d", vidasJogador);
    DrawText(texto, 20, 45, 22, RED);

    sprintf(texto, "Vidas da Base: %d", vidasBase);
    DrawText(texto, 20, 75, 22, RED);

    sprintf(texto, "Recursos: %d", recursos);
    DrawText(texto, 20, 105, 22, RED);

    sprintf(texto, "Monstros Vivos: %d", inimigosVivos);
    DrawText(texto, 20, 135, 22, RED);
}

//função que controla o loop do jogo
void loopJogo(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS], INIMIGO inimigos[MAX_INIMIGOS], BASE *base, int *devePularMenu, int *jogoEmAndamento, int *opcao_pause, int *estadoJogo, int *opcao_menu)
{
    int contador = 0;
    int i, inimigosVivos;

    //enquanto o jogo estiver em andamento
    while (*jogoEmAndamento)
    {
        BeginDrawing();
        ClearBackground(WHITE);
        desenhaMapa(mapa);
        desenhaJogador(jogador, mapa);
        desenhaInimigo(inimigos);
        for(i = 0; i < MAX_INIMIGOS; i++)  //aumenta a quantidade de inimigos vivos
        {
            if(inimigos[i].ativo == 1)
            {
                inimigosVivos++;
            }
        }
        desenhaInfo(jogador->nivel, jogador->vidas, jogador->recursos, base->vidas, inimigosVivos);
        EndDrawing();

        moveJogador(jogador, mapa, inimigos);
        colocaRecurso(jogador, mapa);

        if (IsKeyPressed(KEY_TAB))  //se o jogador apertar tab para pausar o jogo, chama a função de pause
        {
            //menu de Pause
            *opcao_pause = desenhaMenuPause();
            if (*opcao_pause == 1)
            {
                continue; //continuar jogo
            }
            else if (*opcao_pause == 2)
            {
                //carregar jogo salvo
                carregarJogoSalvo("jogo_salvo.bin", jogador, inimigos, base, mapa);
            }
            else if (*opcao_pause == 3)
            {
                //salvar jogo atual
                salvarJogo("jogo_salvo.bin", *jogador, inimigos, *base, mapa);
            }
            else if (*opcao_pause == 4)
            {
                *jogoEmAndamento = 0; //voltar ao menu inicial, encerrando o loop
            }
            else if (*opcao_pause == 5)
            {
                CloseWindow(); //sair do jogo sem salvar
                return 0;
            }
        }

        //contador somente para diminuir a velocidade dos inimigos
        if (contador % 30 == 0)
        {
            moveInimigo(inimigos, mapa, base, jogador);
            contador = 0;
        }
        contador++;
        //se o jogador ou a base ficarem com 0 de vida, estadoJogo fica 0
        if(jogador->vidas <= 0 || base->vidas <= 0)
        {
            *estadoJogo = 0;
            break;
        }
        //se a quantidade de inimigos vivos for igual a 0, passa de nível
        if(inimigosVivos == 0)
        {
            passarNivel(jogador, mapa, inimigos, base, devePularMenu, estadoJogo, opcao_menu, jogoEmAndamento);
        }
        inimigosVivos = 0;
    }
    *jogoEmAndamento = 1;
}

//função que passa o jogador de nível e chama os arquivos
void passarNivel(JOGADOR *jogador, QUADRADO mapa[LINHAS][COLUNAS], INIMIGO inimigos[MAX_INIMIGOS], BASE *base, int *devePularMenu, int *estadoJogo, int *opcao_menu, int *jogoEmAndamento)
{
    int novoNivel = jogador->nivel + 1;
    char nomeArquivo[11];

    sprintf(nomeArquivo, "nivel%d.txt", novoNivel);  //sprintf chama o arquivo seguinte sem a necessidade de um hardcoded
    iniciarJogador(jogador);
    base->vidas = 1;  //vidas da base é inicializado
    carregaMapa(nomeArquivo, mapa, jogador, inimigos, base, devePularMenu, estadoJogo, opcao_menu, jogoEmAndamento);
    //nivel do jogador é atualizado
    jogador->nivel = novoNivel;
}

//função que desenha a tela de game over
int gameOver(int *estadoJogo, int *opcao_menu, int *jogoEmAndamento)
{
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("GAME OVER", 150, 100, 150, RED);
    DrawText("VOLTAR AO MENU [V]", 400, 350, 35, RED);
    DrawText("SAIR [Q]", 400, 550, 35, RED);
    EndDrawing();
    //carrega jogo
    if(IsKeyPressed(KEY_V))
    {
        *estadoJogo = -1;
        *opcao_menu = 0;
        *jogoEmAndamento = 0;
        return 0;
    }
    //sai do jogo
    if(IsKeyPressed(KEY_Q))
    {
        *estadoJogo = -1;
        *opcao_menu = 3;
        return 1;
    }
    return 0;
}

//função que imprime a tela de vitória e dá opção de novo jogo ou saída
int vitoria(int *estadoJogo, int *opcao_menu, int *jogoEmAndamento)
{
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText("VITORIA!", 250, 100, 150, RED);
    DrawText("VOLTAR AO MENU [V]", 400, 350, 35, RED);
    DrawText("SAIR [Q]", 400, 550, 35, RED);
    EndDrawing();

    if(IsKeyPressed(KEY_V))
    {
        *estadoJogo = -1;
        *opcao_menu = 0;
        *jogoEmAndamento = 0;
        return 0;
    }
    if(IsKeyPressed(KEY_Q))
    {
        *estadoJogo = -1;
        *opcao_menu = 3;
        return 1;
    }
    return 0;
}

//inicialização da função main
int main()
{
    int jogoEmAndamento = 1; // Variável para controlar o loop do jogo
    int estadoJogo = -1;
    int opcao_menu = -1;
    int opcao_pause;
    int devePularMenu = 0;

    BASE base;
    base.vidas = 1;
    JOGADOR jogador;
    QUADRADO mapa[LINHAS][COLUNAS];
    INIMIGO inimigos[MAX_INIMIGOS] = {0};

    InitWindow(LARGURA, ALTURA, "TOWER DEFENSE");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if(estadoJogo == 0)
        {
            devePularMenu = gameOver(&estadoJogo, &opcao_menu, &jogoEmAndamento);
            continue;
        }

        if(estadoJogo == 1)
        {
            devePularMenu = vitoria(&estadoJogo, &opcao_menu, &jogoEmAndamento);
            continue;
        }

        if(devePularMenu != 1)
        {
            do {
                opcao_menu = desenhaMenu();
            } while (opcao_menu < 1 || opcao_menu > 3);
        }

        if (opcao_menu == 1)
        {
            //carrega o jogo inicial
            jogoEmAndamento = 1;
            estadoJogo = -1;
            devePularMenu = 0;
            base.vidas = 1;

            iniciarJogador(&jogador);
            carregaMapa("nivel1.txt", mapa, &jogador, inimigos, &base, &devePularMenu, &estadoJogo, &opcao_menu, &jogoEmAndamento);//, &devePularMenu
            loopJogo(&jogador, mapa, inimigos, &base, &devePularMenu, &jogoEmAndamento, &opcao_pause, &estadoJogo, &opcao_menu);
        }
        else if(opcao_menu == 2)
        {
            //carrega o arquivo salvo
            carregarJogoSalvo("jogo_salvo.bin", &jogador, inimigos, &base, mapa);
            loopJogo(&jogador, mapa, inimigos, &base, &devePularMenu, &jogoEmAndamento, &opcao_pause, &estadoJogo, &opcao_menu);
        }
        else if(opcao_menu == 3)
        {
            CloseWindow();
            return 0; //sair do jogo
        }
    }

    CloseWindow();
    return 0;
}

