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
std::string reachTarget = "ReachedTarget";
std::string obsticalString = "AHHH_Theres_an_obsitacal";

std::vector<float> mag_vect;
std::vector<float> dir_vect;

uint32_t vectIndex = 0;

uint64_t getTimeStamp(void ){
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void response_callback(void){

    std::cout << "new vector Requested" <<std::endl;

    if(reachTarget.compare(pathResponse->status)== 0){
        if(vectIndex < mag_vect.size())
        {
            nextPathVector->mag = mag_vect[vectIndex];
            nextPathVector->dir = dir_vect[vectIndex];
            nextPathVector->status = "SendingNextTansition";
            nextPathVector->putMessage();
            std::cout << "sending new vector" << std::endl;
            vectIndex ++;
        } else
        {
            nextPathVector->mag = mag_vect[vectIndex];
            nextPathVector->dir = dir_vect[vectIndex];
            nextPathVector->status = "out of vectors";
            nextPathVector->putMessage();
        }

    }else if (obsticalString.compare(pathResponse->status) == 0){
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

    nextPathVector = new PathVector( new CPJL("localhost", 14000),
                                    "path_vector");

    pathResponse = new PathVector( new CPJL( "localhost", 14000),
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
