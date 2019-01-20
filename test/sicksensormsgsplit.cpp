#include <iostream>
#include <fstream>

#include <SICK_Sensor.h>

using namespace std;

int main(int argc, char* argv[]) {
    SickSensor ss("", 0, false); // dont actually connect to SICK
    ss.scanData("txt/sick-response.txt");

    auto& out = ss.getTokenizedOutput();

    const int STATE_start = 0;
    const int STATE_dist = 1;
    int STATE_current = STATE_start;

    int sz = out.size();
    for(int i = 0; i < sz; i++) {
        auto& str = out[i];
        switch(STATE_current) {
            case STATE_start:
                cout << str << endl;
                if(str == "DIST1") {
                    STATE_current = STATE_dist;
                    i += 4;
                }
                break;

            case STATE_dist:
                try {
                    cout << stoi(str, nullptr, 16) << ' ';
                }
                catch(exception& ex) { // Java ain't the only one with a catch-all
                    cout << str << endl;
                    continue;
                }
                break;

            default:
                break;
        }
    }

    cout << endl;

    return 0;
}
