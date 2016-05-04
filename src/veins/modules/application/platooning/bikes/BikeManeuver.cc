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
            ss << strId.substr(5);
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
        myRole = BIKE;
        vehicleData.lane = -1;
        vehicleData.platoonId = -1;
        vehicleData.speed = 20 / 3.6;
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
            myRole = LEADER;

            vehicleData.lane = platoonLane;
            vehicleData.platoonId = myId;
            vehicleData.speed = 100 / 3.6;
            vehicleData.formation.push_back( 100 );
            vehicleData.formation.push_back( 101 );
            vehicleData.formation.push_back( 102 );
            vehicleData.formation.push_back( 103 );

            //after 30 seconds of simulation, start the maneuver
            startTurn = new cMessage();
            scheduleAt(simTime() + SimTime(10), startTurn);

            break;
        }

        default: {
            //these are the followers which are already in the platoon
            traciVehicle->setCruiseControlDesiredSpeed( 130.0 / 3.6 );
            traciVehicle->setActiveController( Plexe::CACC );
            traciVehicle->setFixedLane( platoonLane );
            myRole = FOLLOWER;

            leaderId = 100;
            frontId = myId - 1;

            vehicleData.frontId = myId - 1;
            vehicleData.lane = platoonLane;
            vehicleData.leaderId = leaderId;
            vehicleData.platoonId = 0;
            vehicleData.speed = 100 / 3.6;
            vehicleData.formation.push_back( 100 );
            vehicleData.formation.push_back( 101 );
            vehicleData.formation.push_back( 102 );
            vehicleData.formation.push_back( 103 );

            break;
        }
        }
    }
}

void BikeManeuver::finish() {

    if (startTurn) {
        cancelAndDelete(startTurn);
        startTurn = 0;
    }

    BaseApp::finish();
}

void BikeManeuver::handleSelfMsg(cMessage * msg)
{
    BaseApp::handleSelfMsg(msg);

    if (msg == startTurn || msg == endTurn)
        handleLeaderMsg(msg);
}

void BikeManeuver::handleLeaderMsg(cMessage * msg)
{
    BikesMessage *toSend;

    //check current leader status
    FSM_Switch(leaderFSM) {
        case FSM_Exit(LS_INIT): {
            FSM_Goto(leaderFSM, LS_IDLE);
            break;
        }
        case FSM_Exit(LS_IDLE): {
            // start the maneuver
            if ( msg == startTurn )
            {
                toSend = generateMessage();
                toSend->setMessageType(LM_TURNING); // send warning beacon!
                sendUnicast(toSend, -1);
                endTurn = new cMessage();
                scheduleAt(simTime() + 10, endTurn);
                FSM_Goto(leaderFSM, LS_TURNING);
            }
            break;
        }
        case FSM_Exit(LS_TURNING):
        {
            if ( msg == endTurn )
            {
                // done turning
                toSend = generateMessage();
                toSend->setMessageType(LM_DONE_TURNING);
                sendUnicast(toSend, -1);
                FSM_Goto(leaderFSM, LS_IDLE);
            }
            break;
        }
    }
}

void BikeManeuver::handleBikeMsg(cMessage * msg)
{
    //this message can be a self message, or a unicast message
    //with an encapsulated beacon or maneuver message
    BikesMessage *maneuver = 0;
    cPacket *encapsulated = 0;
    //maneuver message to be sent, if needed
    BikesMessage *toSend;

    //first check if this is a unicast message, and in case if it is a beacon or a maneuver
    UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
    if (unicast) {
        encapsulated = unicast->decapsulate();
        maneuver = dynamic_cast<BikesMessage *>(encapsulated);
    }

    //check current status
    FSM_Switch(bikeFSM) {
        case FSM_Exit(BS_INIT): {
            FSM_Goto(bikeFSM, BS_GO);
            break;
        }
        case FSM_Exit(BS_GO): {
            if ( maneuver && maneuver->getMessageType() == LM_TURNING )
            {
                //Warning! Car is turning.
                mobility->getVehicleCommandInterface()->setSpeed(0);
                FSM_Goto(bikeFSM, BS_STOP);
            }
            break;
        }
        case FSM_Exit(BS_STOP): {
            if ( maneuver && maneuver->getMessageType() == LM_DONE_TURNING )
            {
                // Car done turning
                mobility->getVehicleCommandInterface()->setSpeed(20/3.6);
                FSM_Goto(bikeFSM, BS_GO);
            }
        }
    }

    if (encapsulated) {
        delete encapsulated;
    }
    if (unicast) {
        delete unicast;
    }

}

void BikeManeuver::handleFollowerMsg(cMessage * msg)
{
    //nothing special for now
    //TODO: Implement warning HERE

    FSM_Switch(followerFSM) {
        case FSM_Exit(FS_INIT): {
            FSM_Goto(followerFSM, FS_IDLE);
            break;
        }
    }
}

BikesMessage * BikeManeuver::generateMessage()
{
    BikesMessage *msg = new BikesMessage();
    msg->setVehicleId(myId);
    msg->setPlatoonId(vehicleData.platoonId);
    msg->setPlatoonLane(vehicleData.lane);
    msg->setPlatoonSpeed(vehicleData.speed);
    return msg;
}

void BikeManeuver::onData(WaveShortMessage *wsm) {
}

void BikeManeuver::handleLowerMsg(cMessage *msg) {
    cMessage *dup = msg->dup();
    BaseApp::handleLowerMsg(msg);
    switch (myRole) {
        case LEADER:
            handleLeaderMsg(dup);
            break;
        case FOLLOWER:
            handleFollowerMsg(dup);
            break;
        case BIKE:
            handleBikeMsg(dup);
            break;
        default:
            ASSERT(false);
            break;
    };
}


void BikeManeuver::receiveMessage(cMessage *msg) {

    switch (myRole) {
        case LEADER:
            handleLeaderMsg(msg);
            break;
        case FOLLOWER:
            handleFollowerMsg(msg);
            break;
        case BIKE:
            handleBikeMsg(msg);
            break;
        default:
            ASSERT(false);
            break;
    };

}

void BikeManeuver::handleLowerControl(cMessage *msg) {
    //lower control message
    UnicastProtocolControlMessage *ctrl = 0;

    ctrl = dynamic_cast<UnicastProtocolControlMessage *>(msg);
    //TODO: check for double free corruption
    if (ctrl) {
        delete ctrl;
    }
    else {
        delete msg;
    }

}

void BikeManeuver::onBeacon(WaveShortMessage* wsm) {
}
