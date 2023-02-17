#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>

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
    return lo + (rand() % (hi - lo));
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

void objetos_init()
{
    int i, j;
    for (i = 0; i < TAM; i++)
    {
        for (j = 0; j < TAM; j++)
        {
            elementos[i][j].tipo = between(1, 5);
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

int main ()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    disp_init();

    //audio_init();

    must_init(al_init_image_addon(), "image");
    sprites_init();

    //hud_init();

    //must_init(al_init_primitives_addon(), "primitives");

    //must_init(al_install_audio(), "audio");
    //must_init(al_init_acodec_addon(), "audio codecs");
    //must_init(al_reserve_samples(16), "reserve samples")

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    keyboard_init();
    //fx_init();
    objetos_init();

    frames = 0;
    score = 0;

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

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
        }

        if(done)
            break;
        
        keyboard_update(&event);

        if(redraw && al_is_event_queue_empty(queue))
        {
            
            disp_pre_draw();
            al_clear_to_color(al_map_rgb(0,0,0));

            objetos_draw();
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
    //hud_deinit();
    //audio_deinit();
    disp_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

}