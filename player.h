typedef struct
{
  uint16_t x;
  uint16_t y;
} POSITION;

typedef struct
{
  POSITION pos;
  float view_angle;
  float fov;
  uint16_t height;
} PLAYER;
