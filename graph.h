#ifndef __GRAPH__
#define __GRAPH__

#define BUFFER_W 320
#define BUFFER_H 240

#define DISP_SCALE 3
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

// ENUM de tipo
#define AMARELO             1
#define AZUL                2
#define VERDE               3
#define VERMELHO            4
#define ROXO                5
#define ROSA                6
#define BRANCO              7
#define ESPECIAL            8

// Posição do Original
#define SX_AMARELO
#define SY_AMARELO
#define SX_VERDE
#define SY_VERDE

// Tamanho do Original
#define WIDTH             101
#define HEIGHT            101

// ENUM de estado
#define NORMAL              1
#define BOMBA               2



typedef struct t_objeto{
    int tipo;
    int estado;
    int x;
    int y;
    int sx;
    int sy;
}