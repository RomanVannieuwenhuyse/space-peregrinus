//Wat hier gebeurt is nogal abstract. Beschouw het eerder als een soort library in plaats van te proberen het te begrijpen, want dat gaat hopelijk niet nodig zijn

//Define binary formats
struct dec_int {
  
    byte arr[2] = {0b00000000, 0b00000000}; 

};

struct dec_long_uint{

    byte arr[4] = {0b00000000, 0b00000000, 0b00000000, 0b00000000};

};

struct dec_float {
  
    byte arr[4] = {0b00000000, 0b00000000, 0b00000000, 0b00000000};

};

//Convert an integer to binary format
struct dec_int i_getBits(int a){
  struct dec_int i_struct;
  int b=0;

  for(int m = 1; m >= 0; m--){
    for(int n = 0; n <= 7; n++){
      bitWrite(i_struct.arr[m], n, bitRead(a, b));
      b++;
    }
  }
  return i_struct;
}

struct dec_long_uint l_u_getBits(long unsigned int a){
  struct dec_long_uint long_u_struct;
  int b = 0;

  for(int m = 3; m >= 0; m--){
    for(int n = 0; n <= 7; n++){
      bitWrite(long_u_struct.arr[m], n, bitRead(a, b));
      b++;
    }
  }
  return long_u_struct;
}

//Convert an float to a binary format
struct dec_float f_getBits(float a){
  struct dec_float f_struct;
  //https://stackoverflow.com/questions/14018894/how-to-convert-float-to-byte-array-of-length-4-array-of-char
  unsigned char const * p = reinterpret_cast<unsigned char const *>(&a);
  for (size_t i = 0; i != sizeof(float); ++i)
  { 
    f_struct.arr[i] = p[i]; //Change order of bytes to match IEEE-754
  }
  return f_struct;
}


void printIntStruct(struct dec_int a, bool bin){
  for (int b = 0; b < 2; b++){
      if (bin) {Serial.println(a.arr[b],BIN);} else
      {Serial.println(a.arr[b]);}
  }
}

void printFloatStruct(struct dec_float a, bool bin){
  for (int b = 0; b < 4; b++){
      if (bin) {Serial.println(a.arr[b],BIN);} else
      {Serial.println(a.arr[b]);}
  }
}
