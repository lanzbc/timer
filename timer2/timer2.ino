void setup(){
   TCCR1A=0; TCCR1B=0;
   // RESET lại 2 thanh ghi
  DDRB |= (1 << PB1);
  // Đầu ra PB1 là OUTPUT ( pin 9)
 
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12)|(1 << WGM13);
    // chọn Fast PWM, chế độ chọn TOP_value tự do  ICR1
    TCCR1A |= (1 << COM1A1);
    // So sánh thường( none-inverting)
    ICR1 = 1;
    // xung răng cưa tràn sau 65535 P_clock
    OCR1A =0.2;
    // Value=16838 -> độ rộng 25 %
    TCCR1B |=(1 << CS11);
    // F_clock/64=16mhz/64=250 khz
    //F_pwm=250khz/65536=3.81469 hz
}
void loop(){
}
