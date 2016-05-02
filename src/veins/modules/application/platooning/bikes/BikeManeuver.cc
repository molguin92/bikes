#include "veins/modules/application/platooning/bikes/BikeManeuver.h"

Define_Module(BikeManeuver);

void BikeManeuver::initialize( int stage )
{
    BaseApp::initialize(stage);

    switch(stage)
    {
    case 0:
        // name finite state machines
        leaderFSM.setName("leaderFSM");
        followerFSM.setName("followerFSM");
        bikeFSM.setName("bikeFSM");
        break;
    case 1:
        //set our type!
        std::string strId = mobility->getExternalId();
        std::stringstream ss;

        if( strId.substr(0, 4).compare("bike") == 0 ){
            // I am a bike
            myType = TBIKE;
            ss << strId.substr(4);
            ss >> myId;
        }
        else if ( strId.substr(0, 3).compare("car") == 0)
        {
            // I am a car
            myType = TCAR;
            ss << strId.substr(4);
            ss >> myId;
            myId += 100; // to avoid ID conflicts between bikes and cars
        }
        else
        {
            std::cerr << "Wrong vehicle type.";
            exit(69);
        }

        UnicastProtocolControlMessage * setMac = new UnicastProtocolControlMessage();
        setMac->setControlCommand(SET_MAC_ADDRESS);
        setMac->setCommandValue(myId);
        sendControlDown(setMac);

        int platoonLane = 0;
        prepareManeuverCars(platoonLane);
        break;
    }
}

void BikeManeuver::prepareManeuverCars( int platoonLane )
{
    if( myType == TBIKE )
    {
        role = BIKE;
        vehicleData.lane = -1;
        vehicleData.platoonId = -1;
        vehicleData.speed = 100 / 3.6;
        vehicleData.formation.push_back( 0 );
    }
    else if( myType == TCAR )
    {
        switch( myId )
        {
            case 100: {
                //this is the leader
                traciVehicle->setCruiseControlDesiredSpeed( 100.0 / 3.6 );
                traciVehicle->setActiveController( Plexe::ACC );
                traciVehicle->setFixedLane( platoonLane );
                role = LEADER;

                vehicleData.lane = platoonLane;
                vehicleData.platoonId = myId;
                vehicleData.speed = 100 / 3.6;
                vehicleData.formation.push_back( 0 );
                vehicleData.formation.push_back( 1 );
                vehicleData.formation.push_back( 2 );
                vehicleData.formation.push_back( 3 );

                break;
            }

            default: {
                //these are the followers which are already in the platoon
                traciVehicle->setCruiseControlDesiredSpeed( 130.0 / 3.6 );
                traciVehicle->setActiveController( Plexe::CACC );
                traciVehicle->setFixedLane( platoonLane );
                role = FOLLOWER;

                leaderId = 0;
                frontId = myId - 1;

                vehicleData.frontId = myId - 1;
                vehicleData.lane = platoonLane;
                vehicleData.leaderId = 0;
                vehicleData.platoonId = 0;
                vehicleData.speed = 100 / 3.6;
                vehicleData.formation.push_back( 0 );
                vehicleData.formation.push_back( 1 );
                vehicleData.formation.push_back( 2 );
                vehicleData.formation.push_back( 3 );

                break;
            }
        }
    }
}
