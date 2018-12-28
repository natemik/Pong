#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

/*
The display is as follows:

(0,0)       320
    --------------------
    |                   |
    |                   |
    |                   | 240
    |                   |
    |                   |
    -------------------- (320,240)
*/

//Defines pins for the display
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

//Sets the pins for the joystick of player 1
const int SW_1_pin = 2; // digital pin connected to switch output
const int X_1_pin = 0; // analog pin connected to X output
const int Y_1_pin = 1; // analog pin connected to Y output

// Sets the pins for the jostick of player 2
const int SW_2_pin = 3;// digital pin connected to switch output
const int X_2_pin = 5;// analog pin connected to X output
const int Y_2_pin = 4;// analog pin connected to Y output

//Variables and constants for the moving rectange (pad)
const uint16_t x_1 = 5; // x position of player 1's paddle
uint16_t y_1 = 100; // initial y position of player 1's paddle
const uint16_t width = 5; // width of each paddle
const uint16_t height = 40; // height of each paddle

const uint16_t x_2 = 311; // x position of player 2's paddle
uint16_t y_2 = 100; // initial y position of player 2's paddle

//Variables and constants for the ball
uint16_t xb = 160; // initial x position of the ball
uint16_t yb = 120; // initial y position of the ball
const uint16_t radius = 5; // radius of the ball
const uint16_t x_bound_ball_lower = 0 + radius; // lower x bound of the screen
const uint16_t x_bound_ball_higher = 320 - radius; // upper x bound of the screen
const uint16_t y_bound_ball_lower = 0 + radius; // lower y bound of the screen
const uint16_t y_bound_ball_higher = 240 - radius; // upper y bound of the screen

//Initial states of the ball in the X and Y
int state_x = 0; //0 in the x is left
int state_y = 8; //8 in the y is 0

//Initial scores for the players
int player_1_score = 0;
int player_2_score = 0;

//Initial state for the menu
int menu_state = 0; //State 0 hovers single player

//Sets player color
bool color_flag_1 = false;
uint16_t color_select_1 = 0; //Sets initial color to BLUE

bool color_flag_2 = false;
uint16_t color_select_2 = 0; //Sets initial color to BLUE

//Array of all colors to use for the pads
uint16_t pad_color [7]= {
    BLUE,
    RED,
    GREEN,
    CYAN,
    MAGENTA,
    YELLOW,
    WHITE,
};

//Bounds for the paddle
const uint16_t y_bound_higher = 240 - height;
const uint16_t y_bound_lower = 0;

// initializing the display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC); 

// function to draw and fill a rectangle, used for paddles
void to_draw(uint16_t x, uint16_t y,uint16_t width,uint16_t height,uint16_t color) {
    tft.drawRect(x,y,width,height,color);
    tft.fillRect(x,y,width,height,color);
}

// function to erase a rectangle, used for paddles
void to_erase(uint16_t x,uint16_t y,uint16_t width,uint16_t height) {
    tft.drawRect(x,y,width,height,BLACK);
    tft.fillRect(x,y,width,height,BLACK);
}

// function to draw the current score of each player on the screen
void draw_score(int player) {
    // if player 1s score needs updating
    if (player == 1) {
        tft.setCursor(100,0);// (100,0) is the position of player 1s score
        to_draw(100,0,15,8,BLACK);// black rectangle to erase previous score
        tft.println(player_1_score);// draw new score
    }
    // if player 2s score needs updating
    else if (player == 2) {
        tft.setCursor(220,0); // (220,0) is the position of player 2s score
        to_draw(220,0,15,8,BLACK);// black rectangle to erase previous score
        tft.println(player_2_score);// draw new score
    }
}

// Intro message to display before game starts
void intro() {
    tft.fillScreen(BLACK);
    tft.setCursor(130,40);// position of first intro line
    tft.println("Welcome!");
    tft.setCursor(47,60);// position of second intro line
    tft.println("First player to 5 points wins the game!");
    tft.setCursor(45,80);// position of third intro line
    tft.println("Please press Player 1 Joystick to begin!");

    while (true) {
        // empty the screen when joystick 1s switch is pressed
        if(digitalRead(SW_1_pin) == 0) {
            tft.fillScreen(BLACK);
            break;
        }
    }
}


// required setup function
void setup() {
    init();

    //Sets up the pins for joystick (Player 1)
    pinMode(SW_1_pin, INPUT);
    digitalWrite(SW_1_pin, HIGH);

    //Sets up the pins for joystick (Player 1)
    pinMode(SW_2_pin, INPUT);
    digitalWrite(SW_2_pin, HIGH);

    //Serial initialization
    Serial.begin(9600);

    //Display initialization
    tft.begin();

    //Rotates so its facing away from the breadboard
    tft.setRotation(3);

    //Sets the background to black
    tft.fillScreen(BLACK);
}


// function returning the joystick value
uint16_t joystick(int Y_pin) {
    uint16_t joy_y = analogRead(Y_pin);
    return joy_y;
}


// function defining the menu and chices made by player
void menu() {
    while (true) {
        // joystick 1 is used for menu navigation
        uint16_t y_pos = joystick(1);
        if(y_pos < 475) {
            // menu state 0 to select single player
            menu_state = 0;
            tft.fillScreen(BLACK);
        }
        else if(y_pos > 600) {
            // menu state 1 to select 2 player
            menu_state = 1;
            tft.fillScreen(BLACK);
        }
        if (menu_state == 0) {
            //show that single player is currently selected
            tft.setCursor(100,50);
            to_draw(100,50,80,8,WHITE);
            tft.setTextColor(BLACK);
            tft.println("Single Player");
            tft.setCursor(100,100);
            tft.setTextColor(WHITE);
            tft.println("Two Player");
        }
        else if (menu_state == 1) {
            // show that 2 player is currently selected
            tft.setCursor(100,50);
            tft.setTextColor(WHITE);
            tft.println("Single Player");
            tft.setCursor(100,100);
            to_draw(100,100,65,8,WHITE);
            tft.setTextColor(BLACK);
            tft.println("Two Player");
        }
        // press on joystick to confirm selection
        if (digitalRead(SW_1_pin) == 0) {
            // empty screen
            tft.fillScreen(BLACK);
            break;
        }
    }
}

//Controls movement of the paddle
void movement(uint16_t x, uint16_t* y, uint16_t Y_pin, uint16_t color_select ) {
    // draw the paddle
    to_draw(x,*y,width,height,pad_color[color_select]);

    // set joy_y to the current joystick value
    uint16_t joy_y = joystick(Y_pin);

    //makes sure the pad is within the bounds
    while((*y >= y_bound_lower) && (*y <= y_bound_higher)) {
        // if paddle is to be moved up
        if(joy_y < 475) {
            // erase paddle
            to_erase(x,*y,width,height);
            if (*y > y_bound_lower) {
                //decrement y value of paddle
                *y = *y - 1;
            }
            // draw paddle at new y position
            to_draw(x,*y,width,height,pad_color[color_select]);
            break;
        }
        // if paddle is to be moved up
        else if(joy_y > 600) {
            // erase the paddle
            to_erase(x,*y,width,height);
            if(*y < y_bound_higher) {
                // increment y value of paddle
                *y = *y + 1;
            }
            // draw the paddle at new y position
            to_draw(x,*y,width,height,pad_color[color_select]);
            break;
        }
        else {
            break;
        }
    }
}


// function to change the colour of the paddle
void change_color(int SW_pin, bool* color_flag, uint16_t* color_select) {
    //bool flag = false;
    if(digitalRead(SW_pin) == 0) {
        if(*color_flag < 1) {
            // select next colour in array
            *color_select = *color_select + 1;
            *color_flag = true;
        }
        if(*color_select > 6) {
            // select initial colour in array
            *color_select = 0;
        }
    }
    else {
        *color_flag = false;
    }
}

// function to draw the ball
void draw_ball() {
    tft.drawCircle(xb,yb,radius,RED);
    tft.fillCircle(xb,yb,radius,RED);
}

// functon to erase the ball
void erase_ball() {
    tft.drawCircle(xb,yb,radius,BLACK);
    tft.fillCircle(xb,yb,radius,BLACK);
}

// function to move the ball on the x direction
// 1 = right, 0 = left
// draw and erase ball each time
void ball_x() {
    switch (state_x) {
        case 1: {
            erase_ball();
            xb++;
            draw_ball();
            break;
        }
        case 0: {
            erase_ball();
            xb--;
            draw_ball();
            break;
        }
    }
}

// function to move the ball in the y direction
// O = up slowest, 2 = up second slowest, 4 = up second fastest, 6 = up fastest
// 1 = down slowest, 3 = down second slowest, 5 = down second fastest, 7 = down fastest
// 8 = no movement in the y direction
// draw and erase ball each time
void ball_y() {
    switch (state_y) {
        case 0: {
            erase_ball();
            yb--;
            draw_ball();
            break;
        }
        case 1: {
            erase_ball();
            yb++;
            draw_ball();
            break;
        }
        case 2: {
            erase_ball();
            yb = yb - 2;
            draw_ball();
            break;
        }
        case 3: {
            erase_ball();
            yb = yb + 2;
            draw_ball();
            break;
        }
        case 4: {
            erase_ball();
            yb = yb - 3;
            draw_ball();
            break;
        }
        case 5: {
            erase_ball();
            yb = yb + 3;
            draw_ball();
            break;
        }
        case 6: {
            erase_ball();
            yb = yb - 4;
            draw_ball();
            break;
        }
        case 7: {
            erase_ball();
            yb = yb + 4;
            draw_ball();
            break;
        }
        case 8: {
            erase_ball();
            draw_ball();
            break;
        }

    }
}


// function to reset ball to center of screen after score
void reset() {
    xb = 160;
    yb = 120;
}


//Controls the movement of the ball
void ball_movement() {
    draw_ball();

    ball_x();
    ball_y();

    while(true) {
        
        // if ball has hit the wall behind player 1
        if(xb <= x_bound_ball_lower) {
            // increment player 2 score and draw
            player_2_score++;
            draw_score(2);
            // short delay
            delay(200);
            erase_ball();
            // reset ball position
            reset();
            // reset y speed
            state_y = 1;
            break;
        }
        // if ball has hit the wall behind player 2
        else if (xb >= x_bound_ball_higher) {
            // increment player 2 score and draw
            player_1_score++;
            draw_score(1);
            // short delay
            delay(200);
            erase_ball();
            // reset ball position
            reset();
            // reset y speed
            state_y = 0;
            break;
        }
        else if (yb <= y_bound_ball_lower) {
            // when ball hits the top of screen, flip y direction while retaining speed
            if (state_y == 0) {
                state_y = 1;
            }
            else if (state_y == 2) {
                state_y = 3;
            }
            else if (state_y == 4) {
                state_y = 5;
            }
            else if (state_y == 6) {
                state_y = 7;
            }
            break;
        }
        else if (yb >= y_bound_ball_higher) {
            // when ball hits the bottom of screen, flip y direction while retianing speed
            if (state_y == 1) {
                state_y = 0;
            }
            else if (state_y == 3) {
                state_y = 2;
            }
            else if (state_y == 5) {
                state_y = 4;
            }
            else if (state_y == 7) {
                state_y = 6;
            }
            break;
        }
        else {
            break;
        }
    }
}

// function to check if the ball has hit a paddle and determine
// what speed to bounce off at
void contact_check(uint16_t x, uint16_t y) {
    /* paddle is as follows

        -----
        | 1 |
        | 2 |
        | 3 |
        | 4 |
        | 5 |
        | 6 |
        | 7 |
        | 8 |
        ----- 
    each of the 8 segments is 5 pixels from top to bottom and the paddle is 5 pixels wide
    if the ball hits segment 1 it bounces upwards at the fastest speed
    if the ball hits segment 2 it bounces upwards at the next fastest speed
    if the ball hits segment 3 it bounces upwards at the second slowest speed
    if the ball hits segment 4 it bounces upwards at the slowest speed
    if the ball hits segment 5 it bounces downwards at the slowest speed
    if the ball hits segment 6 it bounces downwards at the next slowest speed
    if the ball hits segment 7 it bounces downwards at the second fastest speed
    if the ball hits segment 8 it bounces downwards at the fastest speed
    if the ball hits right between segments 4 and 5 it bounces straight off
    */
    if (y == y_1) {
        if(((xb-radius) == (x+width)) && ((yb >= y) && (yb <= y+height))) {
            state_x = 1;

            if ((yb >= y) && (yb <= (y+5))) {
                state_y = 6;
            }
            else if ((yb > (y+5)) && (yb <= (y+10))) {
                state_y = 4;
            }
            else if ((yb > (y+10)) && (yb <= (y+15))) {
                state_y = 2;
            }
            else if ((yb > (y+15)) && (yb < (y+20))) {
                state_y = 0;
            }
            else if (yb == (y+20)) {
                state_y = 8;
            }
            else if ((yb > (y+20)) && (yb <= (y+25))) {
                state_y = 1;
            }
            else if ((yb > (y+25)) && (yb <= (y+30))) {
                state_y = 3;
            }
            else if ((yb > (y+30)) && (yb <= (y+35))) {
                state_y = 5;
            }
            else if ((yb > (y+35)) && (yb <= (y+40))) {
                state_y = 7;
            }
        }
    }
    else if (y == y_2) {
        if(((xb+radius) == x) && ((yb >= y) && (yb <= y+height))) {
            state_x = 0;

            if ((yb >= y) && (yb <= (y+5))) {
                state_y = 6;
            }
            else if ((yb > (y+5)) && (yb <= (y+10))) {
                state_y = 4;
            }
            else if ((yb > (y+10)) && (yb <= (y+15))) {
                state_y = 2;
            }
            else if ((yb > (y+15)) && (yb < (y+20))) {
                state_y = 0;
            }
            else if (yb == (y+20)) {
                state_y = 8;
            }
            else if ((yb > (y+20)) && (yb <= (y+25))) {
                state_y = 1;
            }
            else if ((yb > (y+25)) && (yb <= (y+30))) {
                state_y = 3;
            }
            else if ((yb > (y+30)) && (yb <= (y+35))) {
                state_y = 5;
            }
            else if ((yb > (y+35)) && (yb <= (y+40))) {
                state_y = 7;
            }
        }
    }
}


// function controlling cpu movement for a single player game
void cpu_movement(uint16_t x, uint16_t* y){
    // draw the cpu's paddle
    to_draw(x,*y,width,height,pad_color[0]);

    while((*y >= y_bound_lower) && (*y <= y_bound_higher)){
        // if the ball is above the paddle
        if(yb < (*y + (height/4))){ // height/4 is used to increase difficulty for the player
                                    // by having the cpu aim for one of the edges of it's paddle
            to_erase(x,*y,width,height);
            if(*y > y_bound_lower){
                // decrement the paddle's y value
                *y = *y - 1;
            }
        }
        // if the ball is below the paddle
        else if(yb > *y + + (height/4)){
            to_erase(x,*y,width,height);
            if(*y < y_bound_higher){
                // increment the paddle's y value
                *y = *y + 1;
            }
        }
        // draw the paddle at the new y position
        to_draw(x,*y,width,height,pad_color[0]);
        break;
    }
}


// function displaying a message depending on who wons the game, ie the first to 5 points
void winner(int what_player) {
    // if player 1 wins
    if(what_player == 1) {
        tft.fillScreen(BLACK);
        tft.setCursor(100,40);
        tft.println("Player 1 wins!");
        tft.setCursor(60,60);
        tft.println("Please press the reset button");
        tft.setCursor(70,80);
        tft.println("to return to the main menu");
    }
    // if player 2 wins(specific for 2 player game)
    else if(what_player == 2) {
        tft.fillScreen(BLACK);
        tft.setCursor(100,40);
        tft.println("Player 2 wins!");
        tft.setCursor(60,60);
        tft.println("Please press the reset button");
        tft.setCursor(70,80);
        tft.println("to return to the main menu");        
    }
    // if cpu wins(specific for 1 player game)
    else if(what_player == 3) {
        tft.fillScreen(BLACK);
        tft.setCursor(50,40);
        tft.println("You lost to the CPU!?!?! HAHAHAHA");
        tft.setCursor(60,60);
        tft.println("Please press the reset button");
        tft.setCursor(70,80);
        tft.println("to return to the main menu");
    }
}


// requried main function run on the arduino
int main() {
    setup();
    menu();
    // for a single player game
    if(menu_state == 0) {
        intro();
        while(true) {
            // cpu movement is used
            cpu_movement(x_2,&y_2);
            // only one player movement is used
            movement(x_1,&y_1,Y_1_pin,color_select_1);
            // contact check for both paddles
            contact_check(x_1,y_1);
            contact_check(x_2,y_2);
            // only one player color change
            change_color(SW_1_pin,&color_flag_1,&color_select_1);
            // ball movement control
            ball_movement();
            // score tracking
            draw_score(1);
            draw_score(2);
            // if player socre is 5, end game
            if(player_1_score == 5) {
                winner(1);
                break;
            }
            // if cpu score is 5, end game
            else if(player_2_score == 5) {
                winner(3);
                break;
            }
        }
    }
    // for a 2 player game
    else if(menu_state == 1) {
        tft.setTextColor(WHITE);
        intro();
        while(true) {
            // movement for the 2 paddles
            movement(x_1,&y_1,Y_1_pin,color_select_1);
            movement(x_2,&y_2,Y_2_pin,color_select_2);
            // contact check for each paddle
            contact_check(x_1,y_1);
            contact_check(x_2,y_2);
            // colour change for each paddle
            change_color(SW_1_pin,&color_flag_1,&color_select_1);
            change_color(SW_2_pin,&color_flag_2,&color_select_2);
            // ball movement control
            ball_movement();
            // score tracking
            draw_score(1);
            draw_score(2);
            // if player 1 score is 5, end game
            if(player_1_score == 5) {
                winner(1);
                break;
            }
            // if player 2 socre is 5, end game
            else if(player_2_score == 5) {
                winner(2);
                break;
            }
        }
    }
    return 0;
}