////////////////////////////////////////////////////////////////////////////
// Smart Car System
////////////////////////////////////////////////////////////////////////////

#include <Servo.h> //servo library

#define	MOTOR_RIGHT_FRONT		0x01
#define	MOTOR_RIGHT_REAR		0x02
#define	MOTOR_LEFT_FRONT		0x04
#define	MOTOR_LEFT_REAR			0x08

#define	MOTOR_RIGHT				(MOTOR_RIGHT_FRONT|MOTOR_RIGHT_REAR)
#define	MOTOR_LEFT				(MOTOR_LEFT_FRONT|MOTOR_LEFT_REAR)
#define	MOTOR_FRONT				(MOTOR_RIGHT_FRONT|MOTOR_LEFT_FRONT)
#define	MOTOR_REAR				(MOTOR_RIGHT_REAR|MOTOR_LEFT_REAR)

#define	MOTOR_SPEED				100	// 1-256

#define	MOTOR_DIR_STOP				(0)
#define	MOTOR_DIR_FWD				(1)
#define	MOTOR_DIR_REV				(2)

// state of motor
#define	STATE_MOTOR_STOP			(0)
#define	STATE_MOTOR_MOVING_FORWARD	(1)
#define	STATE_MOTOR_MOVING_BACKWARD	(2)
#define	STATE_MOTOR_TURNING_RIGHT	(3)
#define	STATE_MOTOR_TURNING_LEFT	(4)
#define	STATE_MOTOR_ROTATING_CW		(5)
#define	STATE_MOTOR_ROTATING_CCW	(6)

#define	CTRLMODE_AUTO_DRIVE			(0)
#define	CTRLMODE_MANUAL_DRIVE		(1)
#define	CTRLMODE_LINE_TRACKING		(2)

int LED=13;
volatile int state = LOW;
char getstr;
int in1=6;
int in2=7;
int in3=8;
int in4=9;
int ENA=5;
int ENB=11;
int ABS=100;
int Echo = A4;  
int Trig = A5; 
int rightDistance = 0,leftDistance = 0,middleDistance = 0 ;

Servo myservo; // create servo object to control servo

int	g_state_motor = STATE_MOTOR_STOP;
int g_motor_speed_right;
int g_motor_speed_left;
int	g_ctrl_mode = CTRLMODE_MANUAL_DRIVE;

void _servo_angle(int angle)
{
    myservo.write(angle-25);//setservo position according to scaled value
	// 25 is correction value
}

int _us_get_distance()   
{
	digitalWrite(Trig, LOW);   
	delayMicroseconds(2);
	digitalWrite(Trig, HIGH);  
	delayMicroseconds(20);
	digitalWrite(Trig, LOW);   
	float Fdistance = pulseIn(Echo, HIGH);  
	Fdistance= Fdistance/58;       

	return (int)Fdistance;
}  

void _ctrl_motor_right(int dir, int speed)
{
	analogWrite(ENA, speed);
	g_motor_speed_right = speed;

	if(dir == MOTOR_DIR_FWD) {
		digitalWrite(in1,HIGH);
		digitalWrite(in2,LOW);
	}
	else if(dir == MOTOR_DIR_REV) {
		digitalWrite(in1,LOW);
		digitalWrite(in2,HIGH);
	}
}

void _ctrl_motor_left(int dir, int speed)
{
	analogWrite(ENB, speed);
	g_motor_speed_left = speed;

	if(dir == MOTOR_DIR_FWD) {
		digitalWrite(in3,LOW);
		digitalWrite(in4,HIGH);
	}
	else if(dir == MOTOR_DIR_REV) {
		digitalWrite(in3,HIGH);
		digitalWrite(in4,LOW);
	}
}

void _move_forward(int speed)
{
	_ctrl_motor_right(MOTOR_DIR_FWD, speed);
	_ctrl_motor_left(MOTOR_DIR_FWD, speed);
	
	g_state_motor = STATE_MOTOR_MOVING_FORWARD;
	
	Serial.println("go forward");
}

void _move_backward(int speed)
{
	_ctrl_motor_right(MOTOR_DIR_REV, speed);
	_ctrl_motor_left(MOTOR_DIR_REV, speed);
	
	g_state_motor = STATE_MOTOR_MOVING_BACKWARD;
	
	Serial.println("go backward");
}

void _turn_left(int dir, int speed, int level)
{
	int speed_right = speed + level;
	int speed_left = speed - level;
	if(speed_left < 0) {
		speed_left = 0;
	}
		
	_ctrl_motor_right(dir, speed_right);
	_ctrl_motor_left(dir, speed_left);

	g_state_motor = STATE_MOTOR_TURNING_LEFT;
	
	Serial.println("turn left!");
}

void _turn_right(int dir, int speed, int level)
{
	int speed_right = speed - level;
	int speed_left = speed + level;
	if(speed_right < 0) {
		speed_right = 0;
	}
	_ctrl_motor_right(dir, speed_right);
	_ctrl_motor_left(dir, speed_left);
	
	g_state_motor = STATE_MOTOR_TURNING_RIGHT;
	
	Serial.println("turn right!");
}

void _rotate_ccw(int speed)
{
	_ctrl_motor_right(MOTOR_DIR_FWD, speed);
	_ctrl_motor_left(MOTOR_DIR_REV, speed);
	
	g_state_motor = STATE_MOTOR_ROTATING_CCW;
	
	Serial.println("rotate ccw!");
}

void _rotate_cw(int speed)
{
	_ctrl_motor_right(MOTOR_DIR_REV, speed);
	_ctrl_motor_left(MOTOR_DIR_FWD, speed);
	
	g_state_motor = STATE_MOTOR_ROTATING_CW;
	
	Serial.println("rotate cw!");
}

void _stop()
{
	digitalWrite(ENA,LOW);
	digitalWrite(ENB,LOW);

	g_state_motor = STATE_MOTOR_STOP;

	Serial.println("Stop!");
}

void stateChange()
{
	state = !state;
	digitalWrite(LED, state);  
}

// initialization
void setup()
{ 
	g_ctrl_mode = CTRLMODE_MANUAL_DRIVE;

	myservo.attach(3);// attach servo on pin 3 to servo object

	pinMode(Echo, INPUT);    
	pinMode(Trig, OUTPUT);  
	pinMode(LED, OUTPUT);
	Serial.begin(9600);
	pinMode(in1,OUTPUT);
	pinMode(in2,OUTPUT);
	pinMode(in3,OUTPUT);
	pinMode(in4,OUTPUT);
	pinMode(ENA,OUTPUT);
	pinMode(ENB,OUTPUT);


    _servo_angle(90);//********xxxxx setservo position according to scaled value
    delay(500); 

	_stop();
}

// main loop 
void loop()
{ 


	int dist = _us_get_distance();

	// Stop in case of obstacle
    if(dist <= 20) {
		if((g_state_motor==STATE_MOTOR_MOVING_FORWARD) || 
			(g_state_motor==STATE_MOTOR_TURNING_RIGHT) ||
			(g_state_motor==STATE_MOTOR_TURNING_LEFT)) 
		{
			_stop();
		}
	}

	if(getstr == 'a') {
		g_ctrl_mode = CTRLMODE_AUTO_DRIVE;
		_stop();
	}
	else if(getstr == 'm') {
		stateChange();
		g_ctrl_mode = CTRLMODE_MANUAL_DRIVE;
		_stop();
	}

	getstr = Serial.read();
	if(g_ctrl_mode == CTRLMODE_MANUAL_DRIVE) {
		if(getstr=='f') {
			_move_forward(MOTOR_SPEED);
		}
		else if(getstr=='b') {
			_move_backward(MOTOR_SPEED);
			delay(200);
		}
		else if(getstr=='l') {
			_rotate_ccw(130);
			delay(200);
		}
		else if(getstr=='r') {
			_rotate_cw(130);
			delay(200);
		}
		else if(getstr=='L') {
			_turn_left(MOTOR_DIR_FWD, 120, 70);
			delay(200);
		}
		else if(getstr=='R') {
			_turn_right(MOTOR_DIR_FWD, 120, 70);
			delay(200);
		}
		else if(getstr=='C') {
			_turn_left(MOTOR_DIR_REV, 120, 70);
			delay(200);
		}
		else if(getstr=='D') {
			_turn_right(MOTOR_DIR_REV, 120, 70);
			delay(200);
		}
		else if(getstr=='s') {
			_stop();		 
		}
		else if(getstr=='A') {
			stateChange();
		}
	}
	else if(g_ctrl_mode == CTRLMODE_AUTO_DRIVE) {
		_servo_angle(90);//setservo position according to scaled value
		delay(500); 
		middleDistance = _us_get_distance();

		if(middleDistance<=30) {     
			_stop();
			delay(500); 	  
			_servo_angle(10);  
			delay(1000);      
			rightDistance = _us_get_distance();

			delay(500);
			_servo_angle(90);              
			delay(1000);                                                  
			_servo_angle(180);              
			delay(1000); 
			leftDistance = _us_get_distance();

			delay(500);
			_servo_angle(90);              
			delay(1000);
			if(rightDistance>leftDistance) {
				_rotate_cw(150);
				delay(360);
			}
			else if(rightDistance<leftDistance) {
				_rotate_ccw(150);
				delay(360);
			}
			else if((rightDistance<=30)||(leftDistance<=30)) {
				_move_backward(MOTOR_SPEED);
				delay(180);
			}
			else {
				_move_forward(MOTOR_SPEED);
			}
		}  
		else {
			_move_forward(MOTOR_SPEED);
		}
	}
}



