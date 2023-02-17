#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <time.h>
#include <allegro5/allegro_ttf.h>

long frames;
long score;

void must_init(bool test, const char *description)
{
    if(test) return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

int between(int lo, int hi)
{
    int x;
    do
    {
        x = rand();
    } while (x >= RAND_MAX - (RAND_MAX % (hi - lo)));
    return lo + (x % (hi - lo));
}

float between_f(float lo, float hi)
{
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

#define BUFFER_W 808
#define BUFFER_H 808

#define DISP_SCALE 1
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;

void disp_init()
{
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

    disp = al_create_display(DISP_W, DISP_H);
    must_init(disp, "display");

    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    must_init(buffer, "bitmap buffer");
}

void disp_deinit()
{
    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
}

void disp_pre_draw()
{
    al_set_target_bitmap(buffer);
}

void disp_post_draw()
{
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);

    al_flip_display();
}

#define KEY_SEEN     1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init()
{
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT* event)
{
    switch(event->type)
    {
        case ALLEGRO_EVENT_TIMER:
            for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            key[event->keyboard.keycode] &= KEY_RELEASED;
            break;
    }
}

#define MOUSE_SEEN     1
#define MOUSE_RELEASED 0

ALLEGRO_MOUSE_STATE state;

// Tamanho do Original
#define WIDTH             101
#define HEIGHT            101

// ENUM de estado
#define NORMAL              0
#define BOMBA               1

typedef struct SPRITES
{
    ALLEGRO_BITMAP* _sheet;
    ALLEGRO_BITMAP* amarelo[2];
    ALLEGRO_BITMAP* azul[2];
    ALLEGRO_BITMAP* verde[2];
    ALLEGRO_BITMAP* vermelho[2];
    ALLEGRO_BITMAP* roxo[2];
    ALLEGRO_BITMAP* rosa[2];
    ALLEGRO_BITMAP* branco[2];
    ALLEGRO_BITMAP* especial;
}SPRITES;
SPRITES sprites;

ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h)
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
    sprites._sheet = al_load_bitmap("all_jewels3.png");
    must_init(sprites._sheet, "all jewels");

    sprites.amarelo[0] = sprite_grab(WIDTH*12, HEIGHT*8, WIDTH, HEIGHT);
    sprites.amarelo[1] = sprite_grab(WIDTH*0, HEIGHT*8, WIDTH, HEIGHT);

    sprites.azul[0] = sprite_grab(WIDTH*12, HEIGHT*4, WIDTH, HEIGHT);
    sprites.azul[1] = sprite_grab(WIDTH*0, HEIGHT*4, WIDTH, HEIGHT);

    sprites.verde[0] = sprite_grab(WIDTH*12, HEIGHT*6, WIDTH, HEIGHT);
    sprites.verde[1] = sprite_grab(WIDTH*0, HEIGHT*6, WIDTH, HEIGHT);

    sprites.vermelho[0] = sprite_grab(WIDTH*12, HEIGHT*9, WIDTH, HEIGHT);
    sprites.vermelho[1] = sprite_grab(WIDTH*0, HEIGHT*9, WIDTH, HEIGHT);

    sprites.roxo[0] = sprite_grab(WIDTH*12, HEIGHT*2, WIDTH, HEIGHT);
    sprites.roxo[1] = sprite_grab(WIDTH*0, HEIGHT*2, WIDTH, HEIGHT);

    sprites.rosa[0] = sprite_grab(WIDTH*12, HEIGHT*1, WIDTH, HEIGHT);
    sprites.rosa[1] = sprite_grab(WIDTH*0, HEIGHT*1, WIDTH, HEIGHT);

    sprites.branco[0] = sprite_grab(WIDTH*12, HEIGHT*10, WIDTH, HEIGHT);
    sprites.branco[1] = sprite_grab(WIDTH*0, HEIGHT*10, WIDTH, HEIGHT);

    sprites.especial = sprite_grab(WIDTH*8, HEIGHT*5, WIDTH, HEIGHT);

    // sprite grab no plano de fundo aqui?
}

void sprites_deinit()
{
    al_destroy_bitmap(sprites.amarelo[0]);
    al_destroy_bitmap(sprites.amarelo[1]);

    al_destroy_bitmap(sprites.azul[0]);
    al_destroy_bitmap(sprites.azul[1]);

    al_destroy_bitmap(sprites.verde[0]);
    al_destroy_bitmap(sprites.verde[1]);

    al_destroy_bitmap(sprites.vermelho[0]);
    al_destroy_bitmap(sprites.vermelho[1]);

    al_destroy_bitmap(sprites.roxo[0]);
    al_destroy_bitmap(sprites.roxo[1]);

    al_destroy_bitmap(sprites.rosa[0]);
    al_destroy_bitmap(sprites.rosa[1]);

    al_destroy_bitmap(sprites.branco[0]);
    al_destroy_bitmap(sprites.branco[1]);

    al_destroy_bitmap(sprites.especial);

    al_destroy_bitmap(sprites._sheet);
    // Inserir código pra destruir os bitmaps aqui
    // al_destroy_bitmap(bitmap);
    // no exemplo n recebe parametro pq usa variavel global
}

#define TAM 8

typedef struct ELEMENTO
{
    int tipo;
    int estado;
    int x;
    int y;
} ELEMENTO;
ELEMENTO elementos[TAM][TAM];

bool checa_init(int i, int j)
{
    if (i < 2 && j < 2)
        return true;
    
    int tipo = elementos[i][j].tipo;

    if (i < 2)
    {
        if (tipo != elementos[i][j-1].tipo || tipo != elementos[i][j-1].tipo)
            return true;
        return false;
    }
    if (j < 2)
    {
        if (tipo != elementos[i-1][j].tipo || tipo != elementos[i-2][j].tipo)
            return true;
        return false;
    }
    if ((tipo != elementos[i-1][j].tipo || tipo != elementos[i-2][j].tipo) && (tipo != elementos[i][j-1].tipo || tipo != elementos[i][j-2].tipo))
        return true;
    return false;
}

void objetos_init()
{
    int i, j;
    srand(time(NULL));
    for (i = 0; i < TAM; i++)
    {
        for (j = 0; j < TAM; j++)
        {
            do
            {
                elementos[i][j].tipo = between(1, 5);
            } while (!checa_init(i, j));
            elementos[i][j].estado = NORMAL;
            elementos[i][j].x = i*WIDTH;
            elementos[i][j].y = j*HEIGHT;
        }
    }
}

// ENUM de tipo
#define AMARELO             1
#define AZUL                2
#define VERDE               3
#define VERMELHO            4
#define ROXO                5
#define ROSA                6
#define BRANCO              7
#define ESPECIAL            8

void objetos_draw()
{
    int i, j;
    for (i = 0; i < TAM; i++)
    {
        for (j = 0; j < TAM; j++)
        {
            switch (elementos[i][j].tipo)
            {
                case 0:
                    continue;

                case AMARELO:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.amarelo[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.amarelo[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case AZUL:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.azul[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.azul[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case VERDE:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.verde[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.verde[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case VERMELHO:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.vermelho[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.vermelho[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case ROXO:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.roxo[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.roxo[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case ROSA:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.rosa[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.rosa[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case BRANCO:
                    if (elementos[i][j].estado == NORMAL)
                        al_draw_bitmap(sprites.branco[0], elementos[i][j].x, elementos[i][j].y, 0);
                    else
                        al_draw_bitmap(sprites.branco[1], elementos[i][j].x, elementos[i][j].y, 0);
                    break;

                case ESPECIAL:
                    al_draw_bitmap(sprites.especial, elementos[i][j].x, elementos[i][j].y, 0);
            }
        }
    }
}

void delete_row(int ini, int fim, int col)
{
    int i;
    for (i=ini; i<=fim; i++)
        elementos[i][col].tipo = 0;
}

void delete_col(int ini, int fim, int row)
{
    int i;
    for (i=ini; i<=fim; i++)
        elementos[row][i].tipo = 0;
}

int checa_cima(int tipo, int i, int j)
{
    if (i < 0)
        return 0;
    if (elementos[i][j].tipo != tipo)
        return 0;
    
    return 1 + checa_cima(tipo, i-1, j);
}

int checa_baixo(int tipo, int i, int j)
{
    if (i > TAM)
        return 0;
    if (elementos[i][j].tipo != tipo)
        return 0;
    
    return 1 + checa_baixo(tipo, i+1, j);
}

int checa_esquerda(int tipo, int i, int j)
{
    if (j < 0)
        return 0;
    if (elementos[i][j].tipo != tipo)
        return 0;
    
    return 1 + checa_esquerda(tipo, i, j-1);
}  

int checa_direita(int tipo, int i, int j)
{
    if (j > TAM)
        return 0;
    if (elementos[i][j].tipo != tipo)
        return 0;
    
    return 1 + checa_esquerda(tipo, i, j+1);
}

int checa_vertical(int tipo, int i, int j, int* cima, int* baixo)
{
    int total;
    *cima = checa_cima(tipo, i-1, j);
    *baixo = checa_baixo(tipo, i+1, j);
    total = *cima+*baixo+1;

    if (total < 3)
        return 0;
    
    return total;
}

int checa_horizontal(int tipo, int i, int j, int* esquerda, int* direita)
{
    int total;
    *esquerda = checa_esquerda(tipo, i, j-1);
    *direita = checa_direita(tipo, i, j+1);
    total = *esquerda+*direita+1;

    if (total < 3)
        return 0;
    
    return total;
}

int checa_vertical_recursivo();
int checa_horizontal_recursivo();

int checa_vertical_recursivo(int tipo, int i, int j)
{
    int vertical, cima, baixo, k, points=0;

    vertical = checa_vertical(tipo, i, j, &cima, &baixo);
    if (vertical)
    {
        points += vertical;
        for (k = i-cima; k < i+baixo; k++)
        {
            elementos[k][j].tipo = 0;
            points += checa_horizontal_recursivo(tipo, k, j);
        }
        return points-1;
    }
    return 0;
}

int checa_horizontal_recursivo(int tipo, int i, int j)
{
    int horizontal, esquerda, direita, k, points=0;

    horizontal = checa_horizontal(tipo, i, j, &esquerda, &direita);
    if (horizontal)
    {
        points += horizontal;
        for (k = j-esquerda; k < j+direita; k++)
        {
            elementos[k][j].tipo = 0;
            points += checa_vertical_recursivo(tipo, i, k);
        }
        return points-1;
    }
    return 0;
}

int checa_bloco(int i, int j)
{
    int tipo, points=0;
    tipo = elementos[i][j].tipo;

    points += checa_vertical_recursivo(tipo, i, j);
    points += checa_horizontal_recursivo(tipo, i, j);

    points += 1;

    return points;
}

void troca(int i_ini, int i_fim, int j_ini, int j_fim)
{
    int pos_aux;
    
    pos_aux = elementos[i_ini][j_ini].x;
    elementos[i_ini][j_ini].x = elementos[i_fim][j_fim].x;
    elementos[i_fim][j_fim].x = pos_aux;

    pos_aux = elementos[i_ini][j_ini].y;
    elementos[i_ini][j_ini].y = elementos[i_fim][j_fim].y;
    elementos[i_fim][j_fim].y = pos_aux;

    ELEMENTO aux;
    aux = elementos[i_ini][j_ini];
    elementos[i_ini][j_ini] = elementos[i_fim][j_fim];
    elementos[i_fim][j_fim] = aux;
}

bool tem_movimentos()
{
    int i, j, tem, dump1, dump2;
    
    for (i=0; i<TAM; j++)
    {
        for (j=0; j<TAM; j++)
        {
            if (i < TAM-1)
            {
                troca(i, i+1, j, j);
                tem = checa_vertical(elementos[i][j].tipo, i, j, &dump1, &dump2);
                if (tem)
                {
                    troca(i, i+1, j, j);
                    return 1;
                }
                tem = checa_vertical(elementos[i+1][j].tipo, i+1, j, &dump1, &dump2);
                troca(i, i+1, j, j);
                if (tem)
                    return 1;
            }
            if (j < TAM-1)
            {
                troca(i, i, j, j+1);
                tem = checa_horizontal(elementos[i][j].tipo, i, j, &dump1, &dump2);
                if (tem)
                {
                    troca(i, i, j, j+1);
                    return 1;
                }
                tem = checa_horizontal(elementos[i][j+1].tipo, i, j+1, &dump1, &dump2);
                if (tem)
                    return 1;
            }
        }
    }
    return 0;
}

ALLEGRO_FONT* font;
long score_display = 0;

void menu_init()
{
    bool font_addon = al_init_font_addon();
    must_init(font_addon, "font addon");
    bool ttf_addon = al_init_ttf_addon();
    must_init(ttf_addon, "ttf addon");
    font = al_load_ttf_font("Roboto-Regular.ttf", 48, 0);
    must_init(font, "font");
    score_display = 0;
}

void menu_deinit()
{
    al_destroy_font(font);
}

void draw_menu()
{
    al_draw_text(
        font,
        al_map_rgb(255, 255, 255),
        75, 200,
        0,
        "Jogo feito por Pedro Pasqualini");
}

void calcula_selec(int* i, int* j)
{
    int resto_i, resto_j;
    resto_i = (*i) % 101;
    resto_j = (*j) % 101;
    if (resto_i > 13 && resto_i < 89) 
        *i = (*i) / 101;
    else
        *i = -1;
    if (resto_j > 13 && resto_j < 89)
        *j = (*j) / 101;
    else
        *j = -1;
}

int main ()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_install_mouse(), "mouse");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    disp_init();

    //audio_init();

    must_init(al_init_image_addon(), "image");
    sprites_init();

    menu_init();
    //hud_init();

    //must_init(al_init_primitives_addon(), "primitives");

    //must_init(al_install_audio(), "audio");
    //must_init(al_init_acodec_addon(), "audio codecs");
    //must_init(al_reserve_samples(16), "reserve samples")

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    keyboard_init();
    //fx_init();
    objetos_init();
    

    frames = 0;
    score = 0;

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;
    bool menu = false;
    bool mouse = false;
    int x_clicado;
    int y_clicado;
    int x_segundo;
    int y_segundo;

    al_start_timer(timer);

    while(1)
    {
        al_wait_for_event(queue, &event);

        switch(event.type)
        {
            case ALLEGRO_EVENT_TIMER:
                // mais coisa aqui

                redraw = true;
                frames++;
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;
            
            case ALLEGRO_EVENT_KEY_DOWN:
                if (event.keyboard.keycode == ALLEGRO_KEY_H || event.keyboard.keycode == ALLEGRO_KEY_F1)
                {
                    if (menu == true)
                        menu = false;
                    else
                        menu = true;
                }
            
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                if (event.mouse.button == 1 && !mouse)
                {
                    x_clicado = event.mouse.x;
                    y_clicado = event.mouse.y;
                    calcula_selec(&x_clicado, &y_clicado);
                    if (x_clicado != -1 && y_clicado != -1)
                        mouse=true;
                }
                else if (event.mouse.button == 1)
                {
                    x_segundo = event.mouse.x;
                    y_segundo = event.mouse.y;
                    calcula_selec(&x_segundo, &y_segundo);
                    if (x_segundo != -1 && y_segundo != -1)
                    {
                        troca(x_clicado, x_segundo, y_clicado, y_segundo);
                        mouse=false;
                    }
                }
            //case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                //if (event.mouse.button == 1)
                //mouse=false;
        }

        if(done)
            break;
        
        keyboard_update(&event);

        if(redraw && al_is_event_queue_empty(queue))
        {
            
            disp_pre_draw();
            al_clear_to_color(al_map_rgb(0,0,0));

            objetos_draw();

            if (menu)
                draw_menu();


            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "%02d",x_clicado);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 50, 0, "%02d",y_clicado);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 100, 0, "%02d",x_segundo);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 150, 0, "%02d",y_segundo);
            
            /*
            stars_draw();
            aliens_draw();
            shots_draw();
            fx_draw();
            ship_draw();

            hud_draw();
            */

            disp_post_draw();
            
            redraw = false;

        }
    }

    sprites_deinit();
    menu_deinit();
    //audio_deinit();
    disp_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

}