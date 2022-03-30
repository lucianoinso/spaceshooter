#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "objects.h"
#include <stdio.h>

// ==============================GLOBALS==============================
const int WIDTH = 800;
const int HEIGHT = 400;
const int NUM_BULLETS = 5;
const int NUM_COMETS = 10;
const int NUM_EXPLOSIONS = 5;
const int SCORE_NEEDED_TO_WON = 2000;


enum STATE{TITLE, PLAYING, LOST, WON};
enum KEYS{UP, DOWN, LEFT, RIGHT, SPACE, R_KEY, F_KEY};
bool keys[7] = {false, false, false, false, false, false, false};
bool lifeadded = false;

SpaceShip ship;
Bullet bullets[NUM_BULLETS];
Comet comets[NUM_COMETS];
Explosion explosions[NUM_EXPLOSIONS];

ALLEGRO_SAMPLE *shot = NULL;
ALLEGRO_SAMPLE *boom = NULL;
ALLEGRO_SAMPLE *song = NULL;
ALLEGRO_SAMPLE *gameover = NULL;
ALLEGRO_SAMPLE *wonsound = NULL;
ALLEGRO_SAMPLE_ID wonid;
ALLEGRO_SAMPLE_ID lostid;
ALLEGRO_SAMPLE_INSTANCE *songInstance = NULL;

// Prototypes
void InitShip(SpaceShip &ship, ALLEGRO_BITMAP *image);
void ResetShipAnimation(SpaceShip &ship, int position);
void DrawShip(SpaceShip &ship);
void MoveShipUp(SpaceShip &ship);
void MoveShipDown(SpaceShip &ship);
void MoveShipLeft(SpaceShip &ship);
void MoveShipRight(SpaceShip &ship);

void InitBullet(Bullet bullet[], int size);
void DrawBullet(Bullet bullet[], int size);
void FireBullet(Bullet bullet[], int size, SpaceShip &ship);
void UpdateBullet(Bullet bullet[], int size);
void CollideBullet(Bullet bullet[], int bSize, Comet comets[], int cSize, SpaceShip &ship, Explosion explosion[], int eSize);

void InitComet(Comet comets[], int size, ALLEGRO_BITMAP *image);
void DrawComet(Comet comets[], int size);
void StartComet(Comet comets[], int size);
void UpdateComet(Comet comets[], int size);
void CollideComet(Comet comets[], int cSize, SpaceShip &ship, Explosion explosion[], int eSize);

void InitExplosions(Explosion explosions[], int size, ALLEGRO_BITMAP *image);
void DrawExplosions(Explosion explosions[], int size);
void StartExplosions(Explosion explosions[], int size, int x, int y);
void UpdateExplosions(Explosion explosions[], int size);

void InitBackground(Background &back, float x, float y, float velX, float velY, int width, int height, int dirX, int dirY, ALLEGRO_BITMAP *image);
void UpdateBackground(Background &back);
void DrawBackground(Background &back);

void ChangeState(int &state, int newState);

int main(void)
{

	// Primitive variable
	float gameTime = 0;
	int frames = 0;
	int gameFPS = 0;

	bool done = false;
	bool redraw = true;
	bool showFPS = false;
	const int FPS = 60;

	int state = -1;

	// Object variables
	Background BG;
	Background MG;
	Background FG;

	// Allegro variables
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_FONT *font18 = NULL;
	ALLEGRO_BITMAP *shipImage;
	ALLEGRO_BITMAP *cometImage;
	ALLEGRO_BITMAP *expImage;
	ALLEGRO_BITMAP *title = NULL;
	ALLEGRO_BITMAP *lost = NULL;
	ALLEGRO_BITMAP *won = NULL;
	ALLEGRO_BITMAP *bgImage = NULL;
	ALLEGRO_BITMAP *mgImage = NULL;
	ALLEGRO_BITMAP *fgImage = NULL;

	// Initialization Functions
	if(!al_init())
		return -1;    // Initialize Allegro Library

	display = al_create_display(WIDTH,HEIGHT);

	if(!display)
		return -1;

    al_set_window_position(display, 100, 100);

	al_init_primitives_addon();
	al_install_keyboard();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_image_addon();
	al_install_audio();
	al_init_acodec_addon();

	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FPS);

	shipImage = al_load_bitmap("Spaceship_sprites_by_arboris.png");
	al_convert_mask_to_alpha(shipImage, al_map_rgb(255,0,255));

	cometImage = al_load_bitmap("asteroid-1-96.png");

	expImage = al_load_bitmap("explosion_3_40_128.png");

	title = al_load_bitmap("Shooter_Title.png");
	lost = al_load_bitmap("Shooter_Lose.png");
	won = al_load_bitmap("Shooter_Won.png");

	bgImage = al_load_bitmap("starBG.png");
	mgImage = al_load_bitmap("starMG.png");
	fgImage = al_load_bitmap("starFG.png");

	al_reserve_samples(10);

	shot = al_load_sample("shot.ogg");
	boom = al_load_sample("boom.ogg");
	song = al_load_sample("song.ogg");
	gameover = al_load_sample("dhk.ogg");
	wonsound = al_load_sample("won.wav");

	songInstance = al_create_sample_instance(song);
	al_set_sample_instance_playmode(songInstance, ALLEGRO_PLAYMODE_LOOP);

	al_attach_sample_instance_to_mixer(songInstance, al_get_default_mixer());

	srand(time(NULL));
	
	ChangeState(state, TITLE);
	
	InitShip(ship, shipImage);
	InitBullet(bullets, NUM_BULLETS);
	InitComet(comets, NUM_COMETS, cometImage);
	InitExplosions(explosions, NUM_EXPLOSIONS, expImage);

	InitBackground(BG, 0, 0, 1, 0, 800, 400, -1, 1, bgImage);
	InitBackground(MG, 0, 0, 3, 0, 4000, 400, -1, 1, mgImage);
	InitBackground(FG, 0, 0, 5, 0, 800, 400, -1, 1, fgImage);

	font18 = al_load_font("arial.ttf", 18, 0);

	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_display_event_source(display));

	gameTime = al_current_time();
	al_start_timer(timer);

	while(!done)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_TIMER)
		{
			redraw = true;

			//UPDATE FPS===========
			frames++;
			if(al_current_time() - gameTime >= 1)
			{
				gameTime = al_current_time();
				gameFPS = frames;
				frames = 0;
			}
			//=====================

			if(keys[UP])
				MoveShipUp(ship);
			else if(keys[DOWN])
				MoveShipDown(ship);
			else
				ResetShipAnimation(ship, 1);

			if(keys[LEFT])
				MoveShipLeft(ship);
			else if(keys[RIGHT])
				MoveShipRight(ship);
			else
				ResetShipAnimation(ship, 2);
			
			if(state == TITLE)
			{}
			else if (state == PLAYING)
			{
				UpdateBackground(BG);
				UpdateBackground(MG);
				UpdateBackground(FG);

				UpdateExplosions(explosions, NUM_EXPLOSIONS);
				UpdateBullet(bullets, NUM_BULLETS);
				StartComet(comets, NUM_COMETS);
				UpdateComet(comets, NUM_COMETS);
				CollideBullet(bullets, NUM_BULLETS, comets, NUM_COMETS, ship, explosions, NUM_EXPLOSIONS);
				CollideComet(comets, NUM_COMETS, ship, explosions, NUM_EXPLOSIONS);

				if(ship.lives <= 0)
					ChangeState(state, LOST);

				if(ship.score % 100 == 0 && ship.score != 0 && !lifeadded)
				{
					ship.lives++;
					lifeadded = true;
				}
		
				if(ship.score % 100 == 1)
					lifeadded = false;

				if(ship.score == SCORE_NEEDED_TO_WON)
					ChangeState(state, WON);
					
			}
			else if (state == LOST)
			{}
			else if (state == WON)
			{}

		}
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
		}

		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ESCAPE:
					done = true;
					break;
				case ALLEGRO_KEY_UP:
					keys[UP] = true;
					break;
				case ALLEGRO_KEY_DOWN:
					keys[DOWN] = true;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[LEFT] = true;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[RIGHT] = true;
					break;
				case ALLEGRO_KEY_SPACE:
					keys[SPACE] = true;
					if(state == TITLE)
						ChangeState(state,PLAYING);

					else if(state == PLAYING)
						FireBullet(bullets, NUM_BULLETS, ship);
					break;

				case ALLEGRO_KEY_R:
					keys[R_KEY] = true;
						if(state == LOST || state == WON){
							ChangeState(state, PLAYING);
							InitBackground(BG, 0, 0, 1, 0, 800, 400, -1, 1, bgImage);
							InitBackground(MG, 0, 0, 3, 0, 4000, 400, -1, 1, mgImage);
							InitBackground(FG, 0, 0, 5, 0, 800, 400, -1, 1, fgImage);
						}
					break;
				
				case ALLEGRO_KEY_F:
					keys[F_KEY] = true;
					if(state == PLAYING)
						showFPS = !showFPS;
					break;
			}
		}
		
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ESCAPE:
					done = false;
					break;
				case ALLEGRO_KEY_UP:
					keys[UP] = false;
					break;
				case ALLEGRO_KEY_DOWN: 
					keys[DOWN] = false;
					break;
				case ALLEGRO_KEY_LEFT:
					keys[LEFT] = false;
					break;
				case ALLEGRO_KEY_RIGHT:
					keys[RIGHT] = false;
					break;
				case ALLEGRO_KEY_SPACE:
					keys[SPACE] = false;
					break;
				case ALLEGRO_KEY_F:
					keys[F_KEY] = false;
					break;
			}
		}

		if(redraw && al_is_event_queue_empty(event_queue))
		{
			redraw = false;

			if(state == TITLE)
			{
				al_draw_bitmap(title,0,0,0);
			}
			else if (state == PLAYING)
			{
				DrawBackground(BG);
				DrawBackground(MG);
				DrawBackground(FG);

				DrawShip(ship);
				DrawBullet(bullets, NUM_BULLETS);
				DrawComet(comets, NUM_COMETS);
				DrawExplosions(explosions, NUM_EXPLOSIONS);

				al_draw_textf(font18, al_map_rgb(33,133,200), 5, 5, 0, "Player has %i lives left. Player has destroyed %i objects", ship.lives, ship.score);
				
				if(showFPS)
					al_draw_textf(font18, al_map_rgb(255, 0, 255), WIDTH - 10, 5, ALLEGRO_ALIGN_RIGHT, "FPS: %i", gameFPS);	//display FPS on screen
			}
			else if (state == LOST)
			{
				al_draw_bitmap(lost, 0, 0, 0);				
				al_draw_textf(font18, al_map_rgb(33,133,255), WIDTH - 10, 5, ALLEGRO_ALIGN_RIGHT, "Final Score: %i", ship.score);
			}
			else if (state == WON)
			{
				float accuracy = (float)(ship.score)/ship.firedbullets*100;
				al_draw_bitmap(won, 0, 0, 0);
				al_draw_textf(font18,al_map_rgb(33,133,200), WIDTH - 20 , 5, ALLEGRO_ALIGN_RIGHT,"Shooting accuracy: %.0f %%", accuracy);
			}

			al_flip_display();
			al_clear_to_color(al_map_rgb(0,0,0));
		}
	}
	
	al_destroy_sample(shot);
	al_destroy_sample(boom);
	al_destroy_sample(song);
	al_destroy_sample(gameover);
	al_destroy_sample_instance(songInstance);
	
	al_destroy_bitmap(bgImage);
	al_destroy_bitmap(mgImage);
	al_destroy_bitmap(fgImage);
	
	al_destroy_bitmap(title);
	al_destroy_bitmap(lost);
	al_destroy_bitmap(expImage);
	al_destroy_bitmap(cometImage);
	al_destroy_bitmap(shipImage);
	al_destroy_event_queue(event_queue);
	al_destroy_timer(timer);
	al_destroy_font(font18);
	al_destroy_display(display);

	return 0;
}

void InitShip(SpaceShip &ship, ALLEGRO_BITMAP *image = NULL)
{
	ship.x = 20;
	ship.y = HEIGHT/2;
	ship.ID = PLAYER;
	ship.lives = 3;
	ship.speed = 6;
	ship.boundx = 10;
	ship.boundy = 12;
	ship.score = 0;
	ship.firedbullets = 0;

	ship.maxFrame = 3;
	ship.curFrame = 0;
	ship.frameCount = 0;
	ship.frameDelay = 50;
	ship.frameWidth = 46;
	ship.frameHeight = 41;
	ship.animationColumns = 3;
	ship.animationDirection = 1;

	ship.animationRow = 1;

	if(image !=NULL)
		ship.image = image;
}
void DrawShip(SpaceShip &ship)
{
	int fx = (ship.curFrame % ship.animationColumns) * ship.frameWidth;
	int fy = ship.animationRow * ship.frameHeight;

	al_draw_bitmap_region(ship.image, fx, fy, ship.frameWidth,
		ship.frameHeight, ship.x - ship.frameWidth / 2, ship.y - ship.frameHeight / 2, 0);
	//al_draw_filled_rectangle(ship.x - ship.boundx, ship.y - ship.boundy, ship.x + ship.boundx, 
	//	ship.y + ship.boundy, al_map_rgba(255,0,255,100));

}
void MoveShipUp(SpaceShip &ship)
{
	ship.animationRow = 0;
	ship.y -= ship.speed;
	if(ship.y < 0)
		ship.y = 0;
}
void MoveShipDown(SpaceShip &ship)
{
	ship.animationRow = 2;
	ship.y += ship.speed;
	if(ship.y > HEIGHT)
		ship.y = HEIGHT;

}
void MoveShipLeft(SpaceShip &ship)
{
	ship.curFrame = 2;
	ship.x -= ship.speed;
	if(ship.x < 0)
		ship.x = 0;
}
void MoveShipRight(SpaceShip &ship)
{
	ship.curFrame = 1;
	ship.x += ship.speed;
	if(ship.x > 300)
		ship.x = 300;
}

void InitBullet(Bullet bullet[], int size)
{
	for (int i = 0; i < size; i++)
	{
		bullet[i].ID = BULLET;
		bullet[i].speed = 10;
		bullet[i].live = false;
	}
}

void DrawBullet(Bullet bullet[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if(bullet[i].live)
			al_draw_filled_ellipse(bullet[i].x, bullet[i].y, 5, 1, al_map_rgb(100,100,255)); 
	}
}

void FireBullet(Bullet bullet[], int size, SpaceShip &ship)
{
	for(int i = 0; i < size; i++)
	{
		if(!bullet[i].live)
		{
			bullet[i].x = ship.x + 17;
			bullet[i].y = ship.y;
			bullet[i].live = true;
			ship.firedbullets++;

			al_play_sample(shot, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			break;
		}
	}
}
void UpdateBullet(Bullet bullet[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if(bullet[i].live)
		{
			bullet[i].x += bullet[i].speed;
			if(bullet[i].x > WIDTH)
				bullet[i].live=false;
		}
	}
}
void CollideBullet(Bullet bullet[], int bSize, Comet comets[], int cSize, SpaceShip &ship, Explosion explosion[], int eSize)
{
	for(int i = 0; i < bSize; i++)
	{
		if(bullet[i].live)
		{
			for(int j = 0 ; j< cSize; j++)
			{
				if(comets[j].live)
				{
					if(bullet[i].x > (comets[j].x - comets[j].boundx) &&
						bullet[i].x < (comets[j].x + comets[j].boundx) &&
						bullet[i].y > (comets[j].y - comets[j].boundy) &&
						bullet[i].y < (comets[j].y + comets[j].boundy))
					{
						bullet[i].live = false;
						comets[j].live = false;

						ship.score++;


						StartExplosions(explosion, eSize, bullet[i].x, bullet[i].y);

						al_play_sample(boom, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
					}
				}
			}
		}
	}
}

void InitComet(Comet comets[], int size, ALLEGRO_BITMAP *image = NULL)
{
	for(int i = 0; i < size; i++)
	{
		comets[i].ID = ENEMY;
		comets[i].live = false;
		comets[i].speed = 5;
		comets[i].boundx = 35;
		comets[i].boundy = 35;

		comets[i].maxFrame = 143;
		comets[i].curFrame = 0;
		comets[i].frameCount = 0;
		comets[i].frameDelay = 2;

		comets[i].frameWidth = 96;
		comets[i].frameHeight = 96;
		comets[i].animationColumns = 21;
		comets[i].animationDirection = 1;

		if(rand() % 2)
			comets[i].animationDirection = 1;
		else
			comets[i].animationDirection = -1;

		if(image != NULL)
			comets[i].image = image;
	}
}
void DrawComet(Comet comets[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if(comets[i].live)
		{
			int fx = (comets[i].curFrame % comets[i].animationColumns) * comets[i].frameWidth;
			int fy = (comets[i].curFrame / comets[i].animationColumns) * comets[i].frameHeight;

			al_draw_bitmap_region(comets[i].image, fx, fy, comets[i].frameWidth,
				comets[i].frameHeight, comets[i].x - comets[i].frameWidth / 2, comets[i].y - comets[i].frameHeight /2, 0);

			//al_draw_filled_rectangle(comets[i].x - comets[i].boundx, comets[i].y - comets[i].boundy, comets[i].x + comets[i].boundx, 
			//	comets[i].y + comets[i].boundy, al_map_rgba(255,0,255,100));
		}
	}
}
void StartComet(Comet comets[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if(!comets[i].live)
		{
			if(rand() % 500 == 0)
			{
				comets[i].live = true;
				comets[i].x = WIDTH;
				comets[i].y = 30 + rand() % (HEIGHT - 60);
				break;
			}
		}
	}
}
void UpdateComet(Comet comets[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if(comets[i].live)
		{
			if(++comets[i].frameCount >= comets[i].frameDelay)
			{
				comets[i].curFrame += comets[i].animationDirection;
				if(comets[i].curFrame >= comets[i].maxFrame)
					comets[i].curFrame = 0;
				else if( comets[i].curFrame <= 0)
					comets[i].curFrame = comets[i].maxFrame - 1;

				comets[i].frameCount = 0;
			}
			comets[i].x -= comets[i].speed;
		}
	}
}
void CollideComet(Comet comets[], int cSize, SpaceShip &ship, Explosion explosion[], int eSize)
{
	for(int i = 0; i<cSize; i++)
	{
		if(comets[i].live)
		{
			if(comets[i].x - comets[i].boundx < ship.x + ship.boundx &&
				comets[i].x + comets[i].boundx > ship.x - ship.boundx &&
				comets[i].y - comets[i].boundy < ship.y + ship.boundy &&
				comets[i].y + comets[i].boundy > ship.y - ship.boundy)
			{
				ship.lives--;
				comets[i].live = false;
				StartExplosions(explosion, eSize, ship.x, ship.y);

				al_play_sample(boom, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(comets[i].x < 0)
			{
				comets[i].live = false;
				ship.lives--;
			}
		}
	}
}

void ResetShipAnimation(SpaceShip &ship, int position)
{
	if(position == 1)
		ship.animationRow = 1;
	else
		ship.curFrame = 0;
}

void InitExplosions(Explosion explosions[], int size, ALLEGRO_BITMAP *image = NULL)
{
	for(int i = 0; i< size; i++)
	{
		explosions[i].live = false;

		explosions[i].maxFrame = 31;
		explosions[i].curFrame = 0;
		explosions[i].frameCount = 0;
		explosions[i].frameDelay = 1;
		explosions[i].frameWidth = 128;
		explosions[i].frameHeight = 128;
		explosions[i].animationColumns = 8;
		explosions[i].animationDirection = 1;

		if(image != NULL)
			explosions[i].image = image;
	}
}

void DrawExplosions(Explosion explosions[], int size)
{
	for(int i = 0; i< size; i++)
	{
		if(explosions[i].live)
		{
			int fx = (explosions[i].curFrame % explosions[i].animationColumns) * explosions[i].frameWidth;
			int fy = (explosions[i].curFrame / explosions[i].animationColumns) * explosions[i].frameHeight;

			al_draw_bitmap_region(explosions[i].image, fx, fy, explosions[i].frameWidth,
				explosions[i].frameHeight, explosions[i].x - explosions[i].frameWidth / 2, explosions[i].y - explosions[i].frameHeight /2, 0);
		}
	}
}
void StartExplosions(Explosion explosions[], int size, int x, int y)
{
	for (int i = 0; i < size; i++)
	{
		if(!explosions[i].live)
		{
			explosions[i].live = true;
			explosions[i].x = x;
			explosions[i].y = y;
			break;
		}
	}
}
void UpdateExplosions(Explosion explosions[], int size)
{
	for(int i = 0; i < size; i++)
	{
		if (explosions[i].live)
		{
			if(++explosions[i].frameCount >= explosions[i].frameDelay)
			{
				explosions[i].curFrame += explosions[i].animationDirection;
				if (explosions[i].curFrame >= explosions[i].maxFrame)
				{
					explosions[i].curFrame = 0;
					explosions[i].live = false;
				}
			}
			explosions[i].frameCount = 0;
		}
	}
}

void InitBackground(Background &back, float x, float y, float velx, float vely, int width, int height, int dirX, int dirY, ALLEGRO_BITMAP *image)
{
	back.x = x;
	back.y = y;
	back.velX = velx;
	back.velY = vely;
	back.width = width;
	back.height = height;
	back.dirX = dirX;
	back.dirY = dirY;
	back.image = image;
}
void UpdateBackground(Background &back)
{
	back.x += back.velX * back.dirX;
	if(back.x + back.width <= 0)
		back.x = 0;
}
void DrawBackground(Background &back)
{
	al_draw_bitmap(back.image, back.x, back.y, 0);

	if(back.x + back.width < WIDTH)
			al_draw_bitmap(back.image, back.x + back.width, back.y, 0);
}

void ChangeState(int &state, int newState)
{
	if (state == TITLE)
	{
	
	}
	else if(state == PLAYING)
	{
		al_stop_sample_instance(songInstance);
	}
	else if(state == LOST)
	{
		al_stop_sample(&lostid);
	}
	else if(state == WON)
	{
		al_stop_sample(&wonid);
	}

	state = newState;

	if (state == TITLE)
	{}
	else if(state == PLAYING)
	{
		InitShip(ship);
		InitBullet(bullets, NUM_BULLETS);
		InitComet(comets, NUM_COMETS);
		InitExplosions(explosions, NUM_EXPLOSIONS);

		al_play_sample_instance(songInstance);

	}
	else if(state == LOST)
	{
		al_play_sample(gameover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE,&lostid);
	
	}
	else if(state == WON)
	{
		al_play_sample(wonsound, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, &wonid);
	}
}
