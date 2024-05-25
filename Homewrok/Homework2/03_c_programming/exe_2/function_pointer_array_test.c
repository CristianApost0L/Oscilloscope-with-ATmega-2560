#include <stdio.h>

//declare a function pointer that handles an event

typedef void (*ActionPtrFn)(const unsigned char);

// pointer to function
ActionPtrFn state; // A function can be a *PARAMETER*;

int X;

void s1(const unsigned char c);
void s2(const unsigned char c);
void s3(const unsigned char c);
void s4(const unsigned char c);
void s5(const unsigned char c);


void s1(const unsigned char c) {
  printf("state : s1 , char = %x\n",(char)c);
  if(c==0xAA){
    state = s2;
  }
}

void s2(const unsigned char c) {
  printf("state : s2 , char = %x \n",(char)c);
  if(c!=0x55){
    state = s1;
  }else{
    state = s3;
  }
}

void s3(const unsigned char c) {
  printf("state : s3 , char = %x \n",(char)c);
  X = c;
  state = s4;
}

void s4(const unsigned char c) {
  printf("state : s4 , char = %x \n",(char)c);
  if((int)X/2){
    X--;
  }else{
    state = s5;
  }
}

void s5(const unsigned char c) {
  printf("state : s5 , char = %x \n",(char)c);
  if(c != 0xAA){
    state = s1;
  }else{
    state = s2;
  }
}

int main(int argc, char** argv) {

  state = s1;
  unsigned char v;

  while(1) {
    scanf("%hhx",&v);
    (*state)(v);
  }
}
