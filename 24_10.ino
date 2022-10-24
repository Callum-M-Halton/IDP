int avg = 0; // number of samples for the rolling avg
const int sample_length = 100; // create array to store the last sample_length values for calculating our rolling avg
int vals[sample_length]; // i will count up each loop until sample_length is reached at which point avg can begin being calculated
int i = 0;

void traversing_tunnel(){
    int tunnel_dist = get_ultrasonic_distance(); //NEED TO CONFIGURE SECOND ULTRASONIC SENSOR

    // if complete sample not produced yet
    if (i < sample_length) {
        // add to end of sample 'list'
        vals[i] = tunnel_dist;
        i++;
    } else {
        // new sensor value will form part of new avg
        int tot = tunnel_dist;
        // from second to final element
        for (int j=1; j < sample_length; j++) {
            // add sample value to total and shift element back one index
            tot += vals[j];
            vals[j-1] = vals[j];
        }
    
        // add new sample to newly free spot at end of sample array and find avg
        vals[sample_length - 1] = tunnel_dist;
        avg = tot / sample_length;
    }
    //if the new distance is threshold percentage either side, adjust the speeds to turn slightly
    int tunnel_threshold = 0.05;

    //checks if going closer to the wall
    if (tunnel_dist <= (avg*(1-tunnel_threshold))){
        //going left so need to go right slightly
        state.offset_dir = offset_dirs.left;
        state.offset_ext = offset_exts.little;
		set_motor_speed(true, speeds.med);

    } else if (tunnel_dist >= (avg*(1+tunnel_threshold))){
        //going right so need to go left 
        state.offset_dir = offset_dirs.right;
        state.offset_ext = offset_exts.little;
	    set_motor_speed(false, speeds.med);

    } else {
        //go straight
        state.offset_dir = offset_dirs.none;
        state.offset_ext = offset_exts.none;
		set_motor_speed(false, speeds.high);
        set_motor_speed(true, speeds.high);
    }
    
}


//hall sensor

const byte red_pin = 2;
const byte green_pin = 3;
bool magnetic = 0;
int hallsensor_threshold = 500;
const int HallSensorPin = 5;

PinMode(red_pin, OUTPUT);
PinMode(green_pin, OUTPUT);

void hallSensorTest(){
    // returns value read by hall sensor
    if(analogRead(HallSensorPin) > hallsensor_threshold){
        //box is magnetic
        magnetic = 1; 
                
        //turn on red light for 5 sec if magnetic
        digitalWrite(red_pin, HIGH);
        delay(5000);
        digitalWrite(red_pin, LOW);
    } else {
        //box is not magnetic
        magnetic = 0;

        //turn on green light for 5 sec if not magnetic
        digitalWrite(green_pin, HIGH);
        delay(5000);
        digitalWrite(green_pin, LOW);
    }
}