#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include "player.h"
#include "formulas.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WALL_SIZE 256
#define HALF_WALL_SIZE WALL_SIZE * 0.5
#define MAP_X 64
#define MAP_Y 64

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
PLAYER player;

SDL_FRect img_rect = { .x = 0, .y = 0, .w = 64, .h = 64 };
SDL_FRect floor_rect = { .x = 0, .y = (WINDOW_HEIGHT/2) - 1, .w = WINDOW_WIDTH - 1, .h = WINDOW_HEIGHT - 1 };

SDL_Texture * wall_texture;

uint16_t map[MAP_X][MAP_Y];

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

void drawTextureRect(int32_t x, float height, uint16_t texture_num) {
  SDL_Texture * texture;
  // Just draw _any_ part of the texture to the wall 
  SDL_FRect srcrect = { x = x, .y = 0, .w = 1, .h = height };
  SDL_FRect dstrect = { .x = x, .y = (WINDOW_HEIGHT/2) + (height/2), .w = 1, .h = height };
  
  switch(texture_num) {
    case 0:
      break;
    case 1:
      texture = IMG_LoadTexture(renderer, "Bricks.png");
      SDL_RenderTexture(renderer, texture, &srcrect, &dstrect);
      break;
    case 2:
      break;
    default:
      break;
  }
  SDL_RenderPresent(renderer);
  
}

void drawLine(int32_t x, float height, uint16_t color, bool vertical) {
  int32_t val = 0;
  if(vertical) {
    val = 180;
  }
  else {
    val = 255;
  }

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
  
  SDL_RenderLine(renderer, x, (WINDOW_HEIGHT/2) + (height/2),
                               x, (WINDOW_HEIGHT/2) - (height/2));
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
  bool vertical_wall = false;
  uint32_t x_world, y_world;
 
  for(ray = 0; ray < ray_max; ray++) {
    while(search == true) {
      x = player.pos.x + t*cos(deg2rad(alpha));
      y = player.pos.y + t*sin(deg2rad(alpha));
      
      gridx = unitsToGrid(x);
      gridy = unitsToGrid(y);
      
      x_world = floor(x);
      y_world = floor(y);
        
      if(gridx <= MAP_X && gridx >= 0 && gridy <= MAP_Y && gridy >= 0) {
        if(map[gridx][gridy] > 0) {
          if(min(x_world % 64, (64-x_world) % 64) < min(y_world % 64, (64-y_world) %64)) {
            vertical_wall = true;
          }
          distance = t*cos(deg2rad(alpha - player.view_angle));
          height = (float)(WALL_SIZE)/(float)distance;
          drawLine(ray, height * 1024, map[gridx][gridy], vertical_wall);
          //drawTextureRect(ray, height * 256, map[gridx][gridy]);
          x = player.pos.x;
          y = player.pos.y;
          t = 0;
          search = false;
        }
        else { 
          t+=1; 
        }
        vertical_wall = false;
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
  
  clearScreen();
  castRays(WINDOW_WIDTH);
  
  wall_texture = IMG_LoadTexture(renderer, "./64x/Bricks.png");
  
  if(SDL_RenderTexture(renderer, wall_texture, NULL, &img_rect) == 0) {
      printf("Error loading texture.\n");
  }
  
  SDL_RenderPresent(renderer);
 
  while(SDL_PollEvent(&e) != 0) {
    switch(e.type) {
      case SDL_EVENT_QUIT:
        return false;
      case SDL_EVENT_KEY_DOWN:
        if(e.key.key == SDLK_W) {
          player.pos.y -= 32*sin(deg2rad(player.view_angle - 180));
          player.pos.x -= 32*cos(deg2rad(player.view_angle - 180));
        }
        if(e.key.key == SDLK_S) {
          player.pos.y += 32*sin(deg2rad(player.view_angle - 180));
          player.pos.x += 32*cos(deg2rad(player.view_angle - 180));
        }
        if(e.key.key == SDLK_A) {
          if((player.view_angle--) < 0) { player.view_angle = 360.0; }
        }
        if(e.key.key == SDLK_D) {
          if((player.view_angle++) > 360.0) { player.view_angle = 0.0; }
        }
        break;
      default:
        break;
    }
  }
  return true;
}

bool init(void) {
  uint32_t flags = 0x0000;
  flags |= SDL_INIT_AUDIO;
  flags |= SDL_INIT_VIDEO;
  
  if(SDL_Init(flags) < 0)
    return false;
  
  window = SDL_CreateWindow("Raycast Engine", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if(!window)
    return false;
  else
    printf("SDL_CreateWindow Success\n");

  renderer = SDL_CreateRenderer(window, NULL);
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
  player.pos.y = 100;
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



