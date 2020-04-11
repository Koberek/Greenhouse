


char recBuffer[9];
bool bufferFull;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);


}

void loop() {

  recRPiData();
  printRPiData();

}

void recRPiData(void){
  if (Serial.available() == 9){
    for (int i=0; i<9; i++){
      recBuffer[i] = Serial.read();
    }
    bufferFull = true;
  }
}

void printRPiData(void){
  if (bufferFull){
    for (int i=0; i<9; i++){
      Serial.print(recBuffer[i]);
    }
    Serial.println();
    bufferFull = false;
  }
}
