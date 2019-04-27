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

enum Worm_Status {Idle, Move_Up, Move_Down, Move_Left, Move_Right, Dead };

Worm_Status WormStatus;
int WormLength = 0;
int WormCurrentSpeed = 200000; // 200 ms

int Fruit_Grow_Amount = 0;
int Fruit_Speed_Amount = 0;

#define WORM_MAX_LENGTH 1024

typedef struct
{
	int x;
	int y;
}worm_segment;

static worm_segment WormBody[WORM_MAX_LENGTH];   // 100 parts to worm - as if!! [0] is his head and it fills up

typedef enum {empty,cherry,apple,bananna}fruit_enum;

#define FRUIT_MAX 10
typedef struct
{
	int x;
	int y;
	
	fruit_enum state;
	
	long Created_Tick;
	long Lifetime_ms;
	
	int GrowAmount;
	int SpeedAmount;
	
	bool flash;
	bool flash_state;
	
	int flash_counter;
	
}fruit_item;

static fruit_item FruitList[FRUIT_MAX];    // 100 parts to worm - as if!! [0] is his head and it fills up

void Worm_Reset();
void Worm_Draw();

bool Worm_Check_Point_Inside(int x, int y);
int Fruit_Service();

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
		if (FruitList[index].state != empty)
		{
			counter++;
		}
	}
	
	return counter;
}

bool Fruit_New_Piece()
{	
	bool found_empty = false;
	
	int index; // will be used further down..
	
	for (index = 0; index < FRUIT_MAX; index++)
	{
		if (FruitList[index].state == empty)
		{
			found_empty = true;
			break; // stop looking
		}
	}
	
	if (!found_empty)
	{
		return false;
	}
	
	// make an new x,y location that is not inside the snake or within 1 pixel...
	int temp_x,temp_y;			
	int random_trys = 100;
	bool location_found = false;
			
	while ( (random_trys>0) && (!location_found) )
	{
		temp_x = rand() % matrix.width();
		temp_y = rand() % matrix.height();
		
		if (Worm_Check_Point_Inside(temp_x, temp_y) == 0)
		{
			FruitList[index].x = temp_x;
			FruitList[index].y = temp_y;
					
			location_found = true;										
		}
		
		random_trys--;
	}
	
	if (!location_found)
	{
		return false;
	}
	
	// we need a random number 0 to 2 for 3 types for fruit
	int temp_random = (rand() % 3);			
	switch(temp_random)
	{				
	case 0:
		{
			FruitList[index].state = cherry;
			FruitList[index].Lifetime_ms = 4000;
			FruitList[index].GrowAmount = 1;
			FruitList[index].SpeedAmount = 3;
		}
		break;

	case 1:
		{
			FruitList[index].state = apple;					
			FruitList[index].Lifetime_ms = 6000;
			FruitList[index].GrowAmount = 2;
			FruitList[index].SpeedAmount = 1;
		}
		break;

	case 2:
		{
			FruitList[index].state = bananna;
			FruitList[index].Lifetime_ms = 8000;
			FruitList[index].GrowAmount = 1;
			FruitList[index].SpeedAmount = 1;
		}
		break;
	}
	
	// now the times for this fruit
	FruitList[index].Created_Tick = millis();
	FruitList[index].flash = false;
	FruitList[index].flash_state = true;
	FruitList[index].flash_counter = 0;
				
	return true;
}


int Fruit_Service()
{
	int updates = 0;
	
	// every time a peice is eaten a new peice will spawn random 0 to 3 seconds later...
	
	// need fruit?
	int items = Fruit_Active();
	
	int max_fruit_level = 1;
	
	if (items < max_fruit_level)
	{						
		Fruit_New_Piece();
		updates++;
	}
	
	volatile int index;
	
	for (index = 0; index < FRUIT_MAX; index++)
	{
		if (FruitList[index].state != empty)
		{
			volatile long age_ms = millis() - FruitList[index].Created_Tick;
			
			volatile long ms_left = FruitList[index].Lifetime_ms - age_ms;
			
			if(ms_left <= 0)
			{
				FruitList[index].state = empty;
				updates++;								
			}
			else if (ms_left < 2000)
			{
				FruitList[index].flash_state = true;
				
				FruitList[index].flash_counter++;
				FruitList[index].flash_counter %= 4;

				if (FruitList[index].flash_counter < 2)
				{
					FruitList[index].flash_state = false;
				}
				else
				{
				    FruitList[index].flash_state = true;
				}				
			}						
			else if (ms_left < 4000)
			{
				FruitList[index].flash_counter++;
				FruitList[index].flash_counter %= 6;
				
				if (FruitList[index].flash_counter < 1)
				{
					FruitList[index].flash_state = false;
				}
				else
				{
					FruitList[index].flash_state = true;
				}				
			}						
		}
	}	
	return updates;
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

	for (int index = 0; index < FRUIT_MAX; index++)
	{
		if (FruitList[index].state != empty)
		{
			if ((FruitList[index].x == x)&&(FruitList[index].y == y))
			{
				count++;
			}			
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
			WormStatus = Move_Up;
		}
		break;
		
	case 1:
		{
			WormStatus = Move_Down;
		}
		break;

	case 2:
		{
			WormStatus = Move_Left;			
		}
		break;

	case 3:
		{
			WormStatus = Move_Right;			
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

	switch (WormStatus)
	{
	case Idle:
		{
				
		}
		break;
		
	case Dead:
		{
				
		}
		break;

	case Move_Up:
		{
			new_x = WormBody[WormLength - 1].x;
			new_y = WormBody[WormLength - 1].y - 1;
		}
		break;

	case Move_Down:
		{
			new_x = WormBody[WormLength - 1].x;
			new_y = WormBody[WormLength - 1].y + 1;
		}
		break;

	case Move_Left:
		{
			new_x = WormBody[WormLength - 1].x-1;
			new_y = WormBody[WormLength - 1].y;
		}
		break;

	case Move_Right:
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

void Worm_Check_Dead()
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
		WormStatus = Dead;		
	}
}

void Worm_Move()
{
	if (WormStatus == Dead)
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

	switch (WormStatus)
	{
	case Move_Up:
		{
			WormBody[0].y--;
		}
		break;

	case Move_Down:
		{
			WormBody[0].y++;
		}
		break;

	case Move_Left:
		{
			WormBody[0].x--;
		}
		break;

	case Move_Right:
		{
			WormBody[0].x++;
		}
		break;
		
	default:
		{
			
		}
		break;
	}
	
	// now check if we are eating fruit....
	// go through all the list as elements are not organised neatly - need to check them all
	for (int index = 0; index < FRUIT_MAX; index++)
	{
		if (FruitList[index].state != empty)
		{
			// fruit in this list element...			
			if ((WormBody[0].x == FruitList[index].x)&&(WormBody[0].y == FruitList[index].y))
			{
				// get what it was worth
				Fruit_Grow_Amount += FruitList[index].GrowAmount;
				Fruit_Speed_Amount += FruitList[index].SpeedAmount;
								
				// this fruit is now gone..
				FruitList[index].state = empty;							
			}			
		}
	}	
}

void Worm_Draw()
{
	matrix.fillScreen(matrix.Color333(0, 0, 0));

	for (int index = 0; index < WormLength; index++)
	{
		if (WormStatus == Dead)
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
	
	volatile int index;
	
	for (index = 0; index < FRUIT_MAX ;index++)
	{
		uint16_t color=0;
		switch (FruitList[index].state)
		{
		case cherry:
			{
				color = matrix.Color333(7, 0, 0);
			}
			break;
			
		case bananna:
			{
				color = matrix.Color333(7, 4, 0);
			}
			break;

		case apple:
			{
				color = matrix.Color333(0, 7, 1);				
			}
			break;

		default:
			{
					
			}
			break;
		}
		
		if (FruitList[index].state!=empty)
		{
			if (FruitList[index].flash)
			{
				if (FruitList[index].flash_state)
				{
					matrix.writePixel(FruitList[index].x, FruitList[index].y, color);					
				}
			}
			else
			{
				matrix.writePixel(FruitList[index].x, FruitList[index].y, color);				
			}
		}
	}
	

	if (WormStatus == Dead)
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
            
			if((WormStatus != Move_Up)&&(WormStatus != Move_Down))
			{
				Update = true;
				WormStatus = Move_Up;
				Serial.println("Up");
			}
		}
	}

	if (DownButton.update())
	{
		// somethings hase changed - falling or rising?
		if(DownButton.rose())
		{
			if ((WormStatus != Move_Down)&&(WormStatus != Move_Up))
			{
				// gone low to high - pressed
				Update = true;
				WormStatus = Move_Down;
				Serial.println("Down");
			}
		}
	}

	if (LeftButton.update())
	{
		// somethings hase changed - falling or rising?
		if(LeftButton.rose())
		{
			if ((WormStatus != Move_Left)&&(WormStatus != Move_Right))
			{
				// gone low to high - pressed
				Update = true;
				WormStatus = Move_Left;
				Serial.println("Left");
			}
		}
	}

	if (RightButton.update())
	{
		// somethings hase changed - falling or rising?
		if(RightButton.rose())
		{
			if ((WormStatus != Move_Right)&&(WormStatus != Move_Left))
			{
				// gone low to high - pressed
				Update = true;
				WormStatus = Move_Right;
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

bool Worm_Speed_Up()
{
	int rate = 50;    // 50 = 5% increase in speed			
	if(WormCurrentSpeed > 25000)
	{
		WormCurrentSpeed = (WormCurrentSpeed * 1000) / (1000 + rate);				
		return true;
	}	
	else
	{
		return false;
	}
}

static long millis_last_move;
static long millis_last_fruit;
bool DrawGameOver = false;

int DrawFlags = 0; // we or this value if a draw is needed - like worm direction change or new peice of fruit or dead...

void loop()
{
	if (WormStatus == Idle)
	{
		// waiting for start
		if (Poll_Joystick())
		{
			// button changed
			srand(millis()); // helps randomess
			Worm_Reset();				
			DrawFlags |= 0x0001;
		}
	}
	else if (WormStatus == Dead)
	{
		// worm is dead...
		if (DrawGameOver == 0)
		{
			DrawGameOver = true;
			DrawFlags |= 0x0002;
		}
	}
	else
	{
		// worm should be alive and moving....up down left or right
		if (Poll_Joystick())
		{
			srand(millis());  // helps randomess

			// returns true if there has been an update.. like direction change
			DrawFlags |= 0x0004;
		}

		// keep drawing fruit every 1000ms
		if ((millis() - millis_last_fruit) >= 100)
		{
			millis_last_fruit = millis();
				
			Fruit_Service();
			
			DrawFlags |= 0x0008;
		}
  
		// do we need to move - based on speed setting?
		if ((millis() - millis_last_move) > (WormCurrentSpeed/1000))
		{
			millis_last_move = millis();
			
			// lot going on in move function
			Worm_Move();
			
			// did we get fruit...?
			
			// if so adjust things
			if (Fruit_Grow_Amount > 0)
			{
				Worm_Grow();
				Fruit_Grow_Amount--;
			}
			
			if (Fruit_Speed_Amount > 0)
			{
				Worm_Speed_Up();
				Fruit_Speed_Amount--;
			}

			Worm_Check_Dead();
			DrawFlags |= 0x0010;
		}
	}

	rand();  // keep making random numbers...
	
	if(DrawFlags)
	{
		Worm_Draw();
		DrawFlags = 0;
	}
}
