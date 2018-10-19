#include <Adafruit_Microbit.h>
#include <Adafruit_GFX.h>

Adafruit_Microbit_Matrix microbit;

const int buttonA = 5;
const int buttonB = 11;

// grid dimensions
const uint8_t grid_x = 5;
const uint8_t grid_y = 5;

int player_score = 0;
int player_x = 2;
int player_y = 4;
bool player_left = false;
bool player_right = false;

bool object_alive = false;
int object_x = 0;
int object_x_spawn = 2;
int object_y = 0;
bool randomObjectX = false; // enable/disable random x position

// how many ticks the object waits until falling down
int object_wait_move = 5;
int object_wait_death = 5;
int object_cur = 0; // timer that decreases

uint8_t gamescreen[] =
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

void setup() {
  Serial.begin(9600);
  Serial.println("Microbit is ready?!");

  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  // set initial values
  object_alive = false;
  object_y = 0;
  object_x = object_x_spawn;
  object_cur = object_wait_move;

  // set random seed
  // if nothing connected to this pin,
  // noise will generate a random seed
  randomSeed(analogRead(0));

  microbit.begin();
  microbit.show(microbit.HEART);
  delay(1000);
}

void loop() {

  CheckPlayerMovement();
  ObjectUpdate();
  RenderScene();

  delay(100);
}

void CheckPlayerMovement() {
  if (!digitalRead(buttonA)) {
    if (!player_left) {
      player_left = true;
      Serial.println("Left");
      MoveLeft();
    }
  }
  else if (player_left) {
    player_left = false;
  }
  
  if (!digitalRead(buttonB)) {
    if (!player_right) {
      player_right = true;
      Serial.println("Right");
      MoveRight();
    }
  }
  else if (player_right) {
    player_right = false;
  }
}

void MoveLeft() {
  if (player_x < grid_x-1) {
    player_x++;;
  }
}

void MoveRight() {
  if (player_x > 0) {
    player_x--;
  }
}


void ObjectUpdate() {

  object_cur--;

  if (object_cur <= 0) {
    if (!object_alive) {
      spawnObject();
    }
    else {
      // move object down
      moveObject();
    }
  }
}

void spawnObject() {

  object_cur = object_wait_move;

  // reset y-position
  object_y = 0;

  // randomly select spawn location
  object_x = getRandomXSpawn();
  object_alive = true;
}

void killObject(int playerPoints) {

  player_score += playerPoints;
  object_cur = object_wait_death;
  object_alive = false;
}

void moveObject() {

  object_cur = object_wait_move;

  // randomly move right/left if possible and
  // do not get random location for the last row
  if (randomObjectX && object_y < grid_y - 2) {
    object_x = getRandomX(object_x);
  }

  object_y++;
  if (object_y == player_y && object_x == player_x) {
    // player kills object and gets a point
    killObject(1);
  }
  else if (object_y > grid_y-1) {
    // player didn't catch object
    // TODO: should the player lose?
    killObject(0);
  }
}


// Generate a random value on the x-axis for spawn location.
int getRandomXSpawn() {
  return random(0, grid_x);
}

// Get next or previous value of the given randomly.
int getRandomX(int curX) {
  int dir = -1;
  if (random(0,10) < 5) {
    dir = 1;
  }
  int newX = curX + dir;
  if (newX >= 0 && newX < grid_x) {
    return newX;
  }
  return curX;
}


void RenderScene() {

  for (uint8_t y = 0; y < grid_y; y++) {
    uint8_t curRow = B00000;

    // render player
    if (y == player_y) {
      curRow |= 1 << player_x;
    }

    // render object
    if (y == object_y) {
      curRow |= 1 << object_x;
    }

    gamescreen[y] = curRow;
  }
  
  microbit.show(gamescreen);
  
}
