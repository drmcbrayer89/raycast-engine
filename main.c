#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <math.h>
#include "player.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WALL_SIZE 256
#define MAP_X 64
#define MAP_Y 64

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
PLAYER player;

SDL_Rect ceiling_rect = { .x = 0, .y = 0, .w = WINDOW_WIDTH - 1, .h = (WINDOW_HEIGHT/2) - 1 };
SDL_Rect floor_rect = { .x = 0, .y = (WINDOW_HEIGHT/2) - 1, .w = WINDOW_WIDTH - 1, .h = WINDOW_HEIGHT - 1 };

uint16_t map[MAP_X][MAP_Y];

float deg2rad(float deg) {
  return deg*(M_PI/180.0);
}

void clearScreen(void) {
  SDL_SetRenderDrawColor(renderer, 125, 125, 125, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer, &floor_rect);
}

void generateMap(void) {
  uint16_t x;
  uint16_t y;
  
  // generate outside perimeter (top & bottom)
  for(x = 0; x < MAP_X; x++) {
    map[x][0] = 1;
    map[x][MAP_Y-1] = 1;
  }

  // generate outside perimeter (right and left)
  for(y = 0;y < MAP_Y; y++) {
    map[0][y] = 1;
    map[MAP_X-1][y] = 1;
  }


  for(y = 10; y < 20; y++) {
    map[10][y] = 2;
    map[20][y] = 2;
  }

  for(x = 10; x < 20; x++) {
    map[x][10] = 2;
  }
}

int16_t unitsToGrid(float units) {
  return (int16_t)(floor(units/32));
}

void drawLine(int32_t x, float height, uint16_t color, uint16_t ray) {
  int32_t val;
  float rgb_min = 200;
  float rgb_max = 255;
  // scale ray to color
  float rgb_scaled = (rgb_max-rgb_min) * (((float)ray - 0)/(800 + 0)) + rgb_min;
  
  if(ray <= 300) { val = 128 + (int32_t)rgb_scaled; }
  if(ray > 300 && ray < 500) { val = 255; }
  if(ray >= 500) { val = 255 - (int32_t)rgb_scaled; }

  switch(color) {
    case 1:
      SDL_SetRenderDrawColor(renderer, 0, val, 0, 255);
      break;
    case 2:
      SDL_SetRenderDrawColor(renderer, 0, 0, val, 255);
      break;
    default:
      break;
  }
  SDL_RenderDrawLine(renderer, x, floor((WINDOW_HEIGHT/2) + (height/2)),
                               x, floor((WINDOW_HEIGHT/2) - (height/2)));
}

void castRays(uint16_t ray_max) {
  float alpha = player.view_angle - (player.fov / 2);
  float alpha_delta = (float)player.fov / (float)WINDOW_WIDTH; 
  int16_t ray = 0;
  int16_t t = 0.0;
  float x = (float)player.pos.x;
  float y = (float)player.pos.y;
  float distance; 
  float height;
  int16_t gridx = -1;
  int16_t gridy = -1;
  bool search = true;
 
  for(ray = 0; ray < ray_max; ray++) {
    while(search == true) {
      x = player.pos.x + t*cos(deg2rad(alpha));
      y = player.pos.y + t*sin(deg2rad(alpha));
      
      gridx = unitsToGrid(x);
      gridy = unitsToGrid(y);

      if(gridx <= MAP_X && gridx >= 0 && gridy <= MAP_Y && gridy >= 0) {
        if(map[gridx][gridy] > 0) {
          distance = t*cos(deg2rad(alpha - player.view_angle));
          height = (float)(WALL_SIZE)/(float)distance;
          drawLine(ray, height * 256, map[gridx][gridy], ray);
          x = player.pos.x;
          y = player.pos.y;
          t = 0;
          search = false;
        }
        else { 
          t+=1; 
        }
      }
      else { search = false; } // out of bound?
    }
   // increment alpha
    alpha += alpha_delta;
    if(alpha > (player.view_angle + (player.fov / 2)))
      alpha = player.view_angle - (player.fov / 2);
    search = true;
  }
}

bool loop(void) {
  SDL_Event e;
  // Handle rendering first, then hold for user input
  clearScreen();
  castRays(WINDOW_WIDTH); 
  SDL_RenderPresent(renderer);
  // Update player position/Quit. Dunno if this is correct
  while(SDL_PollEvent(&e) != 0) {
    switch(e.type) {
      case SDL_QUIT:
        return false;
      case SDL_KEYDOWN:
        switch(e.key.keysym.sym) {
          case SDLK_w:
            player.pos.y -= 32*sin(deg2rad(player.view_angle - 180));
            player.pos.x -= 32*cos(deg2rad(player.view_angle - 180));
            break;
          case SDLK_a:
            if((player.view_angle-=2) < 0) { player.view_angle = 360.0; }
            break;
          case SDLK_s:
            player.pos.y += 32*sin(deg2rad(player.view_angle - 180));
            player.pos.x += 32*cos(deg2rad(player.view_angle - 180));
            break;
          case SDLK_d:
            if((player.view_angle+=2) > 360.0) { player.view_angle = 0.0; }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
  return true;
}

bool init(void) {
  if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    return false;
  
  window = SDL_CreateWindow("Raycast Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                                                    800, 600, SDL_WINDOW_SHOWN);
  if(!window)
    return false;
  else
    printf("SDL_CreateWindow Success\n");

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(!renderer)
    return false;
  else
    printf("SDL_CreateRenderer Succeess\n");
  
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  return true;
}

void main(void) {
  player.pos.x = 100;
  player.pos.y = 100;//WINDOW_HEIGHT/2;
  player.view_angle = 90;
  player.fov = 60;

  generateMap();
  // run init & then loop
  if(init()) {
    
    // Wait for event loop to break us out
    while(loop())
    {
      SDL_Delay(10);
    }
  }
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}



