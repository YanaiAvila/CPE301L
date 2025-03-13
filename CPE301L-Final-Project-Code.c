#define F_CPU 8000000UL // 8Mhz
#define BAUD 9600 // Baud rate
#define MYUBRR F_CPU/16/BAUD-1
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


int difficulty_1, difficulty_2, difficulty_3, difficulty_4 = 0; // To select difficulty
int led_position = 0;   // To determine which LED is on
int direction_1 = 1;    // For one direction LED lights up in
int direction_2 = 0;    // For other direction LED lights up in
int p1_score = 0;       // Player 1 score
int p2_score = 0;       // Player 2 score
char p1_score_str[7];   // Player 1 score string
char p2_score_str[7];   // Player 2 score string
uint16_t pot_value = 0; // Potentiometer value


// Initializing USART function
void USART_Init(unsigned int UBRR)
{
    // Set baud rate --> 8Mhz 9600 baud
    UBRR0H = (unsigned char)(UBRR >> 8); // 0000
    UBRR0L = (unsigned char)(UBRR); // 00110011
   
    UCSR0B = (1<<TXEN0); // Enable transmitter (UCSR0B)
   
    // Set 8 bit, 1 stop bit, disabled parity, asynchronous USART
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); // 000000110 --> only 2 bits set
}


void USART_send(unsigned char data){
    while (!(UCSR0A & (1<<UDRE0)));
    // Put data into buffer, copy(send) “data” to UDR0
    UDR0 = data;
}


// Sending function --> Transmit string using UART
void USART_Transmit(char* str)
{
    // Read until  the end of string
    while (*str != 0x00){
        USART_send(*str);
        str++; // Next char
    }
}


// Function to initialize ACD
void ADC_Init() {
     // Set the reference voltage to AVCC (5V)
     // Rest is default (left justified, channel 0)
     ADMUX = (1 << REFS0) | (1 << ADLAR); // 0b 0110 0000
   
    // Enable ADC, start conversion, and set prescaler to 64
    // 0b 11000110
     ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
   
    ADCSRA |=(1<<ADSC); // Start conversion
}


// Function to move the LEDs back and forth
void LED_back_forth(int delay_scale){
    
    // Moves LED in one direction
    if(direction_1){
        led_position++;
        if(led_position >= 8){
            direction_1 = 0;
            direction_2 = 1;
            led_position = 7;
        }
    }
   
    // Moves LED in other direction
    if(direction_2){
        led_position--;
        if(led_position == 0){
            direction_1 = 1;
            direction_2 = 0;
            led_position = 0;
        }
    }
   
    // Shift the LED
    PORTB = (1<< led_position);
   
    // Determine delay based on input
    if(delay_scale == 1){
        _delay_ms(100);
    }
    else if(delay_scale == 2){
        _delay_ms(70);
    }
    else if(delay_scale == 3){
        _delay_ms(50);
    }
    else if(delay_scale == 4){
        _delay_ms(35);
    }
}


// Interrupt service routine for when player 1 presses the button
ISR(INT0_vect) {
    cli();
   
    // If LED 5 is on...    
    if(led_position == PINB4) {
        p1_score++; // Increment player 1 score
        // If the score is 5, print out win output
        if(p1_score == 5){
            itoa(p1_score, p1_score_str, 10); // First convert p1_score to str
            USART_Transmit("Player 1 gains 1 point. Total points: ");
            USART_Transmit(p1_score_str);
            USART_Transmit("\n");
            USART_Transmit("Player 1 Wins!\n");
        }
        else{
            // Else print out that player gains 1 point and total points
            itoa(p1_score, p1_score_str, 10); // First convert p1_score to str
            USART_Transmit("Player 1 gains 1 point. Total points: ");
            USART_Transmit(p1_score_str);
            USART_Transmit("\n");
        }          
    }
    sei();
}


// Interrupt service routine for when player 2 presses the button
ISR(INT1_vect) {
    cli();
   
    // If LED 5 is on...
    if(led_position == PINB4) {
        p2_score++; // Increment player 1 score
        // If the score is 5, print out win output
        if(p2_score == 5){
            itoa(p2_score, p2_score_str, 10); // First convert p2_score to str
            USART_Transmit("Player 2 gains 1 point. Total points: ");
            USART_Transmit(p2_score_str);
            USART_Transmit("\n");
            USART_Transmit("Player 2 Wins!\n");

        }
        else{
            // Else print out that player gains 1 point and total points
            itoa(p2_score, p2_score_str, 10); // First convert p2_score to str
            USART_Transmit("Player 2 gains 1 point. Total points: ");
            USART_Transmit(p2_score_str);
            USART_Transmit("\n");
        }  
    }
    sei();
}


// Interrupt service routine for ADC
ISR(ADC_vect) {
    // Read potentiometer value
    pot_value = ADCH;
}


// Interrupt service routine for timer 1
ISR(TIMER1_COMPA_vect)
{
    cli();

    // Divides the potentiometer for the different difficulties
    if(pot_value < 256){
        difficulty_1 = 1;
        difficulty_2 = 0;
        difficulty_3 = 0;
        difficulty_4 = 0;
    }
    else if(pot_value < 512){
        difficulty_1 = 0;
        difficulty_2 = 1;
        difficulty_3 = 0;
        difficulty_4 = 0;
    }
    else if(pot_value <768){
        difficulty_1 = 0;
        difficulty_2 = 0;
        difficulty_3 = 1;
        difficulty_4 = 0;
    }
    else {
        difficulty_1 = 0;
        difficulty_2 = 0;
        difficulty_3 = 0;
        difficulty_4 = 1;
    }
   
    /* Here I manually set the difficulty since I encountered
       problems trying to implement my potentiometer */
    LED_back_forth(1);
   
    if(difficulty_1){
        LED_back_forth(1);
    }
   
    else if(difficulty_2){
        LED_back_forth(2);
    }
   
    else if(difficulty_3){
        LED_back_forth(3);
    }
   
    else if(difficulty_4){
        LED_back_forth(4);
    }
   
    sei();
}
 

// MAIN PROGRAM
int main(void)
{
    USART_Init(MYUBRR); // Initialize USART
    ADC_Init(); // Initialize ADC
   
    // PORTS
    DDRB |= 0xFF; // port B as LED port (output port)
   
    DDRC = 0; // Make Port C an input for ADC input
    PORTC = 1; // Pull-up resistor
   
    // Set PD2 (INT0) and PD3 (INT1) as inputs with pull-up resistors for buttons
    DDRD &= ~((1 << PIND2) | (1 << PIND3));
    PORTD |= (1 << PIND2) | (1 << PIND3); // Enable pull-up resistors
   
    // Timer
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // Set prescaler to 1024
                                                        // Mode: CTC
    TIMSK1 |= (1 << OCIE1A); // Output compare A match interrupt
    OCR1A = 1;
   
    sei(); // Enable global interrupts
   
    // For interrupts
    EICRA |= (1 << ISC01) | (1 << ISC11); // Falling edge generates interrupt
    EIMSK |= (1 << INT0) | (1 << INT1); // Enable INT0 and INT1 interrupts
   
    while (1)
    {  
        // Runs forever
    }  
}
