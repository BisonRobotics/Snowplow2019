#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <cpp/PathVector.h>
#include <string>
#include <fstream>

using namespace std;

// compiler takes care of showing us where these are
PathVector* nextPathVector_tx = NULL;
PathVector* path_Response_rx = NULL;
bool sendNewVector = false;
bool sendAvoidanceVector =false;
string reachTarget = "ReachedTarget";
string obsticalString = "AHHH_Theres_an_obsitacal";

float[] ob_avoid_mag = {0.75, 1.5 , 0.75};
float[] ob_avoid_right_dir = { 90, -90, -90}
float[] ob_avoid_left_dir =  {-90,  90,  90}

vector<float> mag_vect;
vector<float> dir_vect;

uint32_t vectIndex = 0;

uint64_t getTimeStamp(void ){
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void response_callback(void){

    std::cout << "new vector Requested" <<std::endl;

    if(reachTarget.compare(path_Response_rx->status)== 0){
        if(vectIndex < mag_vect.size())
        {
            nextPathVector_tx->mag = mag_vect[vectIndex];
            nextPathVector_tx->dir = dir_vect[vectIndex];
            nextPathVector_tx->status = "SendingNextTansition";
            nextPathVector_tx->putMessage();
            std::cout << "sending new vector" << std::endl;
            vectIndex ++;
        } else if (obsticalString.compare(path_Response_rx->status) == 0 ){
            cout << path_Response_rx->staus << endl;

            nextPathVector_tx->mag = ob_avoid_mag[0];
            nextPathVector_tx->dir = ob_avoid_right_dir[0];
            nextPathVector_tx->status = "SendingNextTansition";
            nextPathVector_tx->putMessage();
            cout << "sending avoid vector"<< endl;

            mag_vect.insert(vectIndex, ob_avoid_mag[1]);
            mag_vect.insert(vectIndex+1, ob_avoid_mag[2]);
            dir_vect.insert(vectIndex, ob_avoid_right_dir[1]) ;
            dir_vect.insert(vectIndex+1, ob_avoid_right_dir[2]);

            mag_vect.insert(vectIndex+2, nextPathVector_rx->mag ) ;
            dir_vect.insert(vectIndex+2, nextPathVector_rx->dir);

            vectIndex++;
        } else
        {
            nextPathVector_tx->mag = mag_vect[vectIndex];
            nextPathVector_tx->dir = dir_vect[vectIndex];
            nextPathVector_tx->status = "out of vectors";
            nextPathVector_tx->putMessage();
        }

    }else if (obsticalString.compare(path_Response_rx->status) == 0){
        sendAvoidanceVector =true;
    }else{
        sendNewVector = false;
    }

}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        std::cout << "Usage\n  " << argv[0]
        << " <address of CPJL server> <file of vectors>\n";
        return 1;
    }

    std::ifstream vectorFile (argv[1]);

    nextPathVector_tx = new PathVector( new CPJL("localhost", 14000),
                                    "path_vector");

    path_Response_rx = new PathVector( new CPJL( "localhost", 14000),
                                    "path_status",
                                    response_callback);
    float  tmpmag, tmpdir;
    std::string line;
    size_t pos = 0;

    if (vectorFile.is_open()){
        while (std::getline(vectorFile, line)) {
            std::cout << "Vector added to list: ";
            tmpmag = std::stof(line.substr(0, pos = line.find(","))) ;
            std::cout << "magnitude: " << tmpmag<< " " ;
            tmpdir = std::stof(line.substr(pos+1));
            std::cout << "dir: " << tmpdir << std::endl;
            dir_vect.push_back(tmpdir);
            mag_vect.push_back(tmpmag);
        }
    }


    // start the asynch loop
    auto loop = CPJL_Message::loop();
    (void)loop;
    while(true){
        usleep(1000000);
    }
    return 0;
}
