#include <RGBmatrixPanel.h>
#include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2

// P5 RGB Display ouputs
#define CLK   A4
#define OE    8   // TX
#define LAT   A5  // RX
#define A     A0
#define B     A1
#define C     A2
#define D     A3

// Joystick inputs
#define KEY_UP        9
#define KEY_DOWN      10
#define KEY_LEFT      11
#define KEY_RIGHT     12

// RGB DISPLAY SETUP STUFF
// the RGB data pins on featherwing, must be on same PORT as CLK
uint8_t rgbpins[] = { 2, 3, 4, 5, 6, 7 };
// Create a 32-pixel tall matrix with the defined pins
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64, rgbpins);

// our 4 switches that make a joystick
Bounce UpButton;
Bounce DownButton;
Bounce LeftButton;
Bounce RightButton;

int milliseconds_debounce = 15;

enum Worm_Directions {Idle, Up, Down, Left, Right, stopped };

Worm_Directions WormDirection;
int WormLength = 0;
int WormCurrentSpeed = 300000; // 300 ms

#define WORM_MAX_LENGTH 1024

typedef struct
{
	int x;
	int y;
}worm_segment;

static worm_segment WormBody[WORM_MAX_LENGTH];   // 100 parts to worm - as if!! [0] is his head and it fills up

typedef enum {empty,apple,pear,orange}fruit_enum;

#define FRUIT_MAX 10
typedef struct
{
	int x;
	int y;
	
	fruit_enum fruit;
}fruit_item;

static fruit_item FruitList[FRUIT_MAX];    // 100 parts to worm - as if!! [0] is his head and it fills up

void Worm_Reset();
void Worm_Draw();

bool Worm_Check_Point_Inside(int x, int y);

void setup()
{
	//Serial.begin(115200); // COM port speed 115200

	pinMode(KEY_UP, INPUT);
	pinMode(KEY_DOWN, INPUT);
	pinMode(KEY_LEFT, INPUT);
	pinMode(KEY_RIGHT, INPUT);

	UpButton.attach(KEY_UP);
	DownButton.attach(KEY_DOWN);
	LeftButton.attach(KEY_LEFT);
	RightButton.attach(KEY_RIGHT);

	UpButton.interval(milliseconds_debounce);
	DownButton.interval(milliseconds_debounce);
	LeftButton.interval(milliseconds_debounce);
	RightButton.interval(milliseconds_debounce);

	matrix.begin();

	// fix the screen with red
	matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(1, 0, 0));
	delay(500);

	// fix the screen with green
	matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(0, 1, 0));
	delay(500);

	// fix the screen with blue
	matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(0, 0, 1));
	delay(500);

	matrix.fillScreen(matrix.Color333(0, 0, 0));  // 0,0,0 = black
}

int Fruit_Active()
{	
	int counter = 0;
	
	// how may bits of fruit active
	for(int index = 0 ; index < FRUIT_MAX ; index++)
	{
		if (FruitList[index].fruit != empty)
		{
			counter++;
		}
	}
	
	return counter;
}

bool Fruit_New_Piece()
{	
	bool done = false;
	
	for(int index = 0 ; index < FRUIT_MAX ; index++)
	{
		if (FruitList[index].fruit == empty)
		{
			int temp_random = (rand() % 3);
			
			switch (temp_random)
			{				
			case 0:
				{
					FruitList[index].fruit = apple;
				}
				break;

			case 1:
				{
					FruitList[index].fruit = pear;					
				}
				break;

			case 2:
				{
					FruitList[index].fruit = orange;
				}
				break;
			}
			
			// make an new x,y location that is not inside the snake or within 1 pixel...
			int temp_x, temp_y;
			
			while (1)
			{
				temp_x = rand() % matrix.width();
				temp_y = rand() % matrix.height();
			
				if (Worm_Check_Point_Inside(temp_x, temp_y) == 0)
				{
					FruitList[index].x = temp_x;
					FruitList[index].y = temp_y;
					break;
				}				
			}
			break;			
		}
	}
}


void Fruit_Service()
{
	// every time a peice is eaten a new peice will spawn random 0 to 3 seconds later...
	
	// need fruit?
	int items = Fruit_Active();
	
	int max_fruit_level = 5;
	
	if (items < max_fruit_level)
	{						
		Fruit_New_Piece();
	}	
}


bool Worm_Check_Point_Inside(int x, int y)
{
	int count = 0;
	
	for (int index = 0; index < WormLength; index++)
	{
		if( (WormBody[index].x == x)&&(WormBody[index].y==y))
		{
			count++;			
		}
	}	
	return count;
}

void Worm_Reset()
{
	for (int index = 0; index < WORM_MAX_LENGTH; index++)
	{
		WormBody[index].x = 0;
		WormBody[index].y = 0;
	}

	WormLength = 0;


	// now we need a random direction	
	int temp_dir = rand() % 4; // will give us a number 0 to 3.....
	
	switch(temp_dir)
	{
	case 0:
		{
			WormDirection = Up;
		}
		break;
		
	case 1:
		{
			WormDirection = Down;
		}
		break;

	case 2:
		{
			WormDirection = Left;			
		}
		break;

	case 3:
		{
			WormDirection = Right;			
		}
		break;
	}
	
	// draws a worm on the screen in a random location
	// we keep the worm in the middle area all the same...	
	int x_range = matrix.width() / 2;
	int y_range = matrix.height() / 2;
			
	int temp_x = (rand() % x_range) + (x_range / 2);
	int temp_y = (rand() % y_range) + (y_range / 2);
	
	WormBody[0].x = temp_x;
	WormBody[0].y = temp_y;
	
	WormLength++;   // first length of 1...
	
	Worm_Move();
	Worm_Grow();
	Worm_Move();
	Worm_Grow();
		
	Worm_Draw();
}

void Worm_Grow()
{
	int new_x = 0;
	int new_y = 0;

	switch (WormDirection)
	{
	case Idle:
		{
				
		}
		break;
		
	case stopped:
		{
				
		}
		break;

	case Up:
		{
			new_x = WormBody[WormLength - 1].x;
			new_y = WormBody[WormLength - 1].y - 1;
		}
		break;

	case Down:
		{
			new_x = WormBody[WormLength - 1].x;
			new_y = WormBody[WormLength - 1].y + 1;
		}
		break;

	case Left:
		{
			new_x = WormBody[WormLength - 1].x-1;
			new_y = WormBody[WormLength - 1].y;
		}
		break;

	case Right:
		{
			new_x = WormBody[WormLength - 1].x+1;
			new_y = WormBody[WormLength - 1].y;
		}
		break;
	}

	WormBody[WormLength].x = new_x;
	WormBody[WormLength].y = new_y;

	WormLength++;
}

void CheckDead()
{
	volatile int flags = 0;
	
	if (WormBody[0].y >= matrix.height())
	{
		flags |= 0x0001;
	}

	if (WormBody[0].y < 0)
	{
		flags |= 0x0002;
	}

	if (WormBody[0].x >= matrix.width())
	{
		flags |= 0x0004;
	}

	if (WormBody[0].x < 0)
	{
		flags |= 0x0008;
	}

	for (int index = 1; index < WormLength; index++)
	{
		if ((WormBody[0].x == WormBody[index].x) && (WormBody[0].y == WormBody[index].y))
		{
			flags |= 0x0010;
		}
	}
	
	if (flags != 0)
	{		
		WormDirection = stopped;		
	}
}

void Worm_Move()
{
	if (WormDirection == stopped)
	{
		return;
	}

	int index = WormLength - 1;
	// take from the tail and push all the values down the array
	while(index > 0)
	{
		WormBody[index].x = WormBody[index - 1].x;
		WormBody[index].y = WormBody[index - 1].y;
		index--;
	}

	switch (WormDirection)
	{
	case Up:
		{
			WormBody[0].y--;
		}
		break;

	case Down:
		{
			WormBody[0].y++;
		}
		break;

	case Left:
		{
			WormBody[0].x--;
		}
		break;

	case Right:
		{
			WormBody[0].x++;
		}
		break;
	}
	
	for (int index = 0; index < Fruit_Active(); index++)
	{
		if ((WormBody[0].x == FruitList[index].x)&&(WormBody[0].y == FruitList[index].y))
		{
			// head is eating fruit....!
			FruitList[index].fruit = empty;
			Worm_Grow();
			
			int rate = 20; // 20 = 2% increase in speed
			
			if (WormCurrentSpeed > 25000)
			{
				WormCurrentSpeed = (WormCurrentSpeed * 1000) / (1000 + rate) ;
			}			
			break;
		}
	}	

	CheckDead();
}

void Worm_Draw()
{
	matrix.fillScreen(matrix.Color333(0, 0, 0));

	for (int index = 0; index < WormLength; index++)
	{
		if (WormDirection == stopped)
		{
			matrix.writePixel(WormBody[index].x, WormBody[index].y, matrix.Color333(1, 1, 1));
		}
		else
		{
			if (index == 0)
			{
				matrix.writePixel(WormBody[index].x, WormBody[index].y, matrix.Color333(0, 0, 7));
			}
			else
			{
/*				uint16_t color = 0;
				switch ((index - 1) % 4)
				{

				case 2:
				case 0:
					color = matrix.Color333(0, 7, 0);
					break;

				case 1:
					color = matrix.Color333(0, 0, 7);
					break;

				
				case 3:
					color = matrix.Color333(5, 5, 5);
					break;
				}*/
				
				int color = matrix.Color333(1, 1, 1);
				matrix.writePixel(WormBody[index].x, WormBody[index].y, color);
			}
			
			// draw the fruit			
		}						
	}  
	
	for (int index = 0; index < Fruit_Active(); index++)
	{
		uint16_t color=0;
		switch (FruitList[index].fruit)
		{
		case apple:
			{
				color = matrix.Color333(7, 0, 0);
			}
			break;
			
		case orange:
			{
				color = matrix.Color333(7, 4, 0);
			}
			break;

		case pear:
			{
				color = matrix.Color333(0, 7, 1);				
			}
			break;

		default:
			{
					
			}
			break;
		}
		
		if (color > 0)
		{
			matrix.writePixel(FruitList[index].x, FruitList[index].y, color);			
		}
	}
	

	if (WormDirection == stopped)
	{
		matrix.setTextSize(1);      // size 1 == 8 pixels high
		matrix.setTextWrap(false);  // Don't wrap at end of line - will do ourselves
		matrix.setTextColor(matrix.Color333(7, 0, 0));

		char temp[200];
    
		sprintf(temp, "L:%d", WormLength);
		matrix.setCursor(8, 8);
		matrix.println(temp);
     
		sprintf(temp, "S:%d", WormCurrentSpeed/1000);
		matrix.setCursor(8, 16);
		matrix.println(temp);    
	}
}

bool Poll_Joystick()
{
	bool Update = false;

	if (UpButton.update())
	{
		// somethings hase changed - falling or rising?
		if(UpButton.rose())
		{
			// gone low to high - pressed
            
			if((WormDirection != Up)&&(WormDirection != Down))
			{
				Update = true;
				WormDirection = Up;
				Serial.println("Up");
			}
		}
	}

	if (DownButton.update())
	{
		// somethings hase changed - falling or rising?
		if(DownButton.rose())
		{
			if ((WormDirection != Down)&&(WormDirection != Up))
			{
				// gone low to high - pressed
				Update = true;
				WormDirection = Down;
				Serial.println("Down");
			}
		}
	}

	if (LeftButton.update())
	{
		// somethings hase changed - falling or rising?
		if(LeftButton.rose())
		{
			if ((WormDirection != Left)&&(WormDirection != Right))
			{
				// gone low to high - pressed
				Update = true;
				WormDirection = Left;
				Serial.println("Left");
			}
		}
	}

	if (RightButton.update())
	{
		// somethings hase changed - falling or rising?
		if(RightButton.rose())
		{
			if ((WormDirection != Right)&&(WormDirection != Left))
			{
				// gone low to high - pressed
				Update = true;
				WormDirection = Right;
				Serial.println("Right");
			}
		}
	}

	return Update;
}

void debug_keys()
{
	Serial.print("KEY_UP=");
	Serial.print(digitalRead(KEY_UP));
	Serial.print(",KEY_DOWN=");
	Serial.print(digitalRead(KEY_DOWN));
	Serial.print(",KEY_LEFT=");
	Serial.print(digitalRead(KEY_LEFT));
	Serial.print(",KEY_RIGHT=");
	Serial.print(digitalRead(KEY_RIGHT));
	Serial.println();
}

static long millis_last_move;
static long millis_last_fruit;
bool WaitForMove = false;
bool DrawDead = false;

void loop()
{
	if (WormDirection == Idle)
	{
		if (Poll_Joystick())
		{
			// button changed
			Worm_Reset();				
		}
	}
	else
	{
		if (WormDirection != stopped)
		{
			if (Poll_Joystick())
			{
				Worm_Draw();
				WaitForMove = true;
			}

			if ((millis() - millis_last_fruit) >= 1000)
			{
				millis_last_fruit = millis();
				
				Fruit_Service();
			}
  
			if ((millis() - millis_last_move) > (WormCurrentSpeed/1000))
			{
				millis_last_move = millis();
				Worm_Move();
				Worm_Draw();
				WaitForMove = false;    
			}
		}
		else
		{
			if (DrawDead == 0)
			{
				Worm_Draw();
				DrawDead = true;
			}
		}				
	}

	rand();  // keep making random numbers...
}
