#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <cpp/PathVector.h>
#include <string>
#include <fstream>

// compiler takes care of showing us where these are
PathVector* nextPathVector = NULL;
PathVector* pathResponse = NULL;
bool sendNewVector = false;
bool sendAvoidanceVector =false;
std::string reachTarget = "ReachedTarget;SendNewVector";
std::string obsticalString = "AHHH_Theres_an_obsitacal";

uint64_t getTimeStamp(void ){
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void response_callback(void){

    if(reachTarget.compare(pathResponse->status)== 0){
        sendNewVector= true;
    }else if (obsticalString.compare(pathResponse->status) == 0){
        sendAvoidanceVector =true;
    }else{
        sendNewVector = false;
    }
}

int main(int argc, char* argv[]) {

    if(argc != 3) {
        std::cout << "Usage\n  " << argv[0]
        << " <address of CPJL server> <file of vectors>\n";
        return 1;
    }

    std::ifstream vectorFile (argv[2]);

    nextPathVector = new PathVector( new CPJL(argv[1], 14000),
                                    "path_vector");

    pathResponse = new PathVector( new CPJL( "localhost", 14000),
                                    "path_status",
                                    response_callback);
    float  mag, dir;
    std::string line;
    size_t pos = 0;
    sendNewVector = true;

    if (vectorFile.is_open()){
        while (std::getline(vectorFile, line)) {
            std::cout << "New Vector: ";
            mag = std::stof(line.substr(0, pos = line.find(","))) ;
            std::cout << "magnitude: " << mag<< " " ;
            dir = std::stof(line.substr(pos+1));
            std::cout << "dir: " << dir << std::endl;

            while(!sendNewVector){
                usleep(100000); // wait for status message
            }

            nextPathVector->mag = mag;
            nextPathVector->dir = dir;
            nextPathVector->status = "SendingNextTansition";
            nextPathVector->putMessage();
            sendNewVector =false;

        }
    }

    return 0;
}
