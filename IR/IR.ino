// Reserve space for 10 entries in the average bucket.
// Change the type between < and > to change the entire way the library works.
int val;
void setup() {
    Serial.begin(9600);
}

void loop() {
    // Add a new random value to the bucket
    val = 10 + (27.728*1.3 * pow(map(analogRead(A0), 0, 1023, 0, 5000) / 1000.0, -1.2045));
    Serial.println(val);
    delay(100);
}
