#include <Adafruit_Microbit.h>
//#include <Adafruit_GFX.h>

Adafruit_Microbit_Matrix microbit;

const int buttonA = 5;
const int buttonB = 11;

// grid dimensions
const uint8_t grid_x = 5;
const uint8_t grid_y = 5;

int player_score = 0;
int player_x = 0;
int player_y = 0;
bool player_left = false;
bool player_right = false;
bool showScore = false;
bool game_end = false;

uint8_t decrease_after = 2;
int last_score_decrease = 0;

bool object_alive = false;
int object_x = 0;
int object_y = 0;
bool randomObjectX = false; // enable/disable random x position

// how many ticks the object waits until falling down
int object_wait_move = 0;
const int object_wait_move_init = 50;
const int object_wait_death = 50;
const int object_wait_killed = 100; // how long to wait if player killed it
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

  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  // set random seed
  // if nothing connected to this pin,
  // noise will generate a random seed
  randomSeed(analogRead(0));

  microbit.begin();
  Serial.println("Microbit ready!");
  delay(1000);

  // set initial values
  resetGame(true);
}

void loop() {

  // wait for user to press a button to restart
  if (game_end) {
    if (!digitalRead(buttonA) || !digitalRead(buttonB)) {
      resetGame(false);
      delay(1000);
    }
  }
  else {
    // update player and object
    if (!showScore) {
      CheckPlayerMovement();
      ObjectUpdate();
    }
  }
  
  // render scene
  RenderScene();
  delay(10);
}


// reset the game to initial state
void resetGame(bool firstStart) {

  object_y = 0;
  object_x = 2;
  object_wait_move = object_wait_move_init; // initial value
  object_cur = object_wait_move;
  object_alive = false;

  player_x = 2;
  player_y = 4;
  player_score = 0;
  player_left = false;
  player_right = false;
  
  showScore = false;
  
  if (firstStart) {
    microbit.show(microbit.HEART);
    game_end = true;
  }
  else {
    game_end = false;
    Serial.println("Game reset!");
    
    microbit.clear();
    delay(500);
    
    // count down
    for (uint8_t i = 0; i < 3; i++) {
      microbit.print(3-i);
      delay(1000);  
    }
    microbit.clear();
  }
}


// check if the player wants to move
void CheckPlayerMovement() {
  if (!digitalRead(buttonA)) {
    if (!player_left) {
      player_left = true;
      MoveLeft();
    }
  }
  else if (player_left) {
    player_left = false;
  }
  
  if (!digitalRead(buttonB)) {
    if (!player_right) {
      player_right = true;
      MoveRight();
    }
  }
  else if (player_right) {
    player_right = false;
  }
}


// make player move left
void MoveLeft() {
  if (player_x < grid_x-1) {
    player_x++;;
  }
}


// make player move right
void MoveRight() {
  if (player_x > 0) {
    player_x--;
  }
}


// update object position and status
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


// spawn the object
void spawnObject() {

  object_cur = object_wait_move;

  // reset y-position
  object_y = 0;

  // randomly select spawn location
  object_x = getRandomXSpawn();

  // increase speed using eulers number
  // (remove less from wait time the more the score increases)
  int remove_wait = speedWaitRemove(player_score);
  if (player_score > 0 && remove_wait > 0) {
    object_wait_move -= remove_wait;
    Serial.print("Decreasing wait time with ");
    Serial.print(remove_wait);
    Serial.print(" at ");
    Serial.println(object_wait_move);
  }

  object_alive = true;
  Serial.println("New object spawned!");
}


// Get how much to remove from the wait time depending on player score
// to increase the current object movement speed
int speedWaitRemove(int current_score) {
  
  // decrease each x points if greater than minimum
  if (current_score - last_score_decrease > decrease_after-1) {
    last_score_decrease = current_score;

    if (object_wait_move > object_wait_move_init * 0.8) {
      return 5;
    }

    // wait longer until increasing speed
    decrease_after = 3;

    if (object_wait_move > object_wait_move_init * 0.5) {
      randomObjectX = true;
      return 2;
    }

    // wait longer until increasing speed
    decrease_after = 4;
    
    if (object_wait_move > 5) {
      randomObjectX = false;
      return 1;
    }
  }
  
  return 0;
}


// called when the player hits the object
void killObject(int playerPoints) {

  Serial.println("Object catched!");
  player_score += playerPoints;
  object_cur = object_wait_death;
  object_alive = false;
  
  if (playerPoints > 0) {
    object_cur = object_wait_killed;
    delay(20);

    // catch object sequence
    for (int y = player_y; y >= 0; y--) {
      uint8_t curRow = gamescreen[y];
      curRow |= 1 << player_x;
      gamescreen[y] = curRow;
      microbit.show(gamescreen);
      delay(20);
    }
  }
}


// called when the player did not catch the object
void killPlayer() {

  Serial.println("GAME OVER!");
  object_alive = false;

  // play end sequence
  bool dir = true;
  for (uint8_t y = 0; y < grid_y; y++) {
    uint8_t curRow = gamescreen[y];
    
    for (uint8_t x = 0; x < grid_x; x++) {
      uint8_t shiftVal = x;
      if (!dir) { shiftVal = grid_x - x - 1; }
      curRow |= 1 << shiftVal;
      gamescreen[y] = curRow;
      microbit.show(gamescreen);
      delay(50);
    }

    // change direction
    dir = !dir;
  }
  delay(100);

  // flashing effect
  for (uint8_t i = 0; i < 3; i++) {
    microbit.clear();
    delay(100);
    microbit.show(gamescreen);
    delay(100);
  }

  // finally clear and wait a moment
  microbit.clear();
  delay(500);

  // show score until player presses a button to reset
  showScore = true;
}


// update object position
void moveObject() {

  object_cur = object_wait_move;

  // randomly move right/left if possible and
  // do not get random location for the last 2 rows
  if (randomObjectX && object_y < grid_y - 3) {
    object_x = getRandomX(object_x);
  }

  object_y++;
  if (object_y == player_y && object_x == player_x) {
    
    // player kills object and gets a point
    killObject(1);
  }
  else if (object_y > grid_y-1) {
    // player didn't catch object
    killPlayer();
  }
}


// Generate a random value on the x-axis for spawn location.
int getRandomXSpawn() {
  return random(0, grid_x);
}


// Get next or previous value of the given randomly.
int getRandomX(int curX) {
  int dir = 0;
  int rndNum = random(0,15);
  if (rndNum < 10) { dir = 1; }
  if (rndNum < 5) { dir = -1; }
  int newX = curX + dir;
  if (newX >= 0 && newX < grid_x) {
    return newX;
  }
  return curX;
}


// render the player, object and score
void RenderScene() {

  // show player score
  if (showScore) {
    microbit.print(player_score);

    // game over
    if (!game_end) {
      game_end = true;
    }
  }

  // don't render the scene if game is over
  if (game_end) {
    delay(100);
    return;
  }

  // clear game screen
  for (uint8_t y = 0; y < grid_y; y++) {
    uint8_t curRow = B00000;
    gamescreen[y] = curRow;
  }

  // add player position
  if (player_y > -1 && player_y < grid_y) {
    gamescreen[player_y] = gamescreen[player_y] | 1 << player_x;
  }

  // add object position
  if (object_alive && object_y > -1 && object_y < grid_y) {
    gamescreen[object_y] = gamescreen[object_y] | 1 << object_x;
  }

  // show the scene
  microbit.show(gamescreen);
}
