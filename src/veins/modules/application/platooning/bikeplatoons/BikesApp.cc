//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "veins/modules/application/platooning/bikeplatoons/BikesApp.h"


//#include "crng.h"
//#include "WaveShortMessage_m.h"
//#include "MacPkt_m.h"
//#include "Mac1609_4.h"
//
//#include <BaseProtocol.h>

Define_Module( BikesApp );

void BikesApp::initialize( int stage )
{
	BaseApp::initialize( stage );

	if( stage == 0 )
	{
		//name the FSMs
		leaderFsm.setName( "leaderFsm" );
		followerFsm.setName( "followerFsm" );
		leaverFsm.setName( "leaverFsm" );
		bikeFsm.setName( "bikeFsm" );
	}

	if( stage == 1 )
	{
		//first change id depending on necessity
		std::string strId = mobility->getExternalId();
		std::stringstream ss;
		if( strId.substr( 0, 8 ).compare( "platoon." ) == 0 )
		{
			myType = _CAR;
			ss << strId.substr( 4 );
			ss >> myId;
		}

		if( strId.substr( 0, 5 ).compare( "turn." ) == 0 )
		{
			myType = _CAR;
			ss << strId.substr( 5 );
			ss >> myId;
		}

		if( strId.substr( 0, 5 ).compare( "bike." ) == 0 )
		{
			myType = _BIKE;
			ss << strId.substr( 5 );
			ss >> myId;
		}
		//tell our MAC address to the unicast protocol
		UnicastProtocolControlMessage *setMac =
		    new UnicastProtocolControlMessage();
		setMac->setControlCommand( SET_MAC_ADDRESS );
		setMac->setCommandValue( myId );
		sendControlDown( setMac );

		//lane where the platoon is driving
		int platoonLane = 0;
		prepareManeuverCars( platoonLane );
	}
}

void BikesApp::prepareManeuverCars( int platoonLane )
{
	if( myType == _BIKE )
	{
		role = BIKE;
		vehicleData.lane = -1;
		vehicleData.platoonId = -1;
		vehicleData.speed = 100 / 3.6;
		vehicleData.formation.push_back( 0 );
	}
	else if( myType == _CAR )
	{
		switch( myId )
		{
			case 0: {
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

void BikesApp::finish()
{
	if( startManeuver )
	{
		cancelAndDelete( startManeuver );
		startManeuver = 0;
	}

	BaseApp::finish();
}

BikesMessage *BikesApp::generateMessage()
{
	BikesMessage *msg = new BikesMessage();

	msg->setVehicleId( myId );
	msg->setPlatoonId( vehicleData.platoonId );
	msg->setPlatoonLane( vehicleData.lane );
	msg->setPlatoonSpeed( vehicleData.speed );
	return msg;
}

void BikesApp::onData( WaveShortMessage *wsm )
{
}

void BikesApp::handleSelfMsg( cMessage *msg )
{
	//this takes car of feeding data into CACC and reschedule the self message
	BaseApp::handleSelfMsg( msg );
//
//    if (msg == startManeuver)
//        handleJoinerMsg(msg);
}

void BikesApp::handleLowerMsg( cMessage *msg )
{
	cMessage *dup = msg->dup();
	BaseApp::handleLowerMsg( msg );

//    switch (role) {
//    case LEADER:
//        handleLeaderMsg(dup);
//        break;
//    case FOLLOWER:
//        handleFollowerMsg(dup);
//        break;
//    case JOINER:
//        handleJoinerMsg(dup);
//        break;
//    default:
//        ASSERT(false);
//        break;
//    };
	handleMsg( dup );
}

void BikesApp::receiveMessage( cMessage *msg )
{
//  switch (role) {
//    case LEADER:
//        handleLeaderMsg(msg);
//        break;
//    case FOLLOWER:
//        handleFollowerMsg(msg);
//        break;
//    case LEAVER:
//        handleLeaverMsg(msg);
//        break;
//    case BIKE:
//        handleBikeMsg(msg);
//        break;
//    default:
//        ASSERT(false);
//        break;
//    };
	handleMsg( msg );
}



void BikesApp::handleMsg( cMessage *msg )
{
	std::cout << "msg TO ";
	std::cout << myId;
	std::cout << "\n";
	BikesMessage *toSend = generateMessage();

	toSend->setMessageType( BS_IDLE );
	sendUnicast( toSend, vehicleData.leaderId );
	FSM_Goto( bikeFsm, BS_STOP );
}


//
////TODO: Add broadcast alerts as possibility of msg
//void BikesApp::handleLeaderMsg(cMessage *msg) {	//this message can be a self message, or a unicast message
//    //with an encapsulated beacon or maneuver message
//
//
//    ManeuverMessage *maneuver = 0;
//    cPacket *encapsulated = 0;
//    //maneuver message to be sent, if needed
//    ManeuverMessage *toSend;
//
//    //first check if this is a unicast message, and in case if it is a beacon or a maneuver
//    UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
//    if (unicast) {
//        encapsulated = unicast->decapsulate();
//        maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
//    }
//
//    //check current leader status
//    FSM_Switch(leaderFsm)
//    {
//    case FSM_Exit(LS_INIT): {
//        FSM_Goto(leaderFsm, LS_LEADING);
//        break;
//    }
//    case FSM_Exit(LS_LEADING): {
//        //when getting a message, and being in the LEADING state, we need
//        //to check if this is a join request. if not just ignore it
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//            if (maneuver->getMessageType() == JM_REQUEST_JOIN) {
//
//                toSend = generateMessage();
//                toSend->setMessageType(LM_MOVE_IN_POSITION);
//                //this will be the front vehicle for the car which will join
//                toSend->setFrontVehicleId(3);
//                //save some data. who is joining?
//                vehicleData.joinerId = maneuver->getVehicleId();
//                //send a positive ack to the joiner
//                sendUnicast(toSend, vehicleData.joinerId);
//
//                FSM_Goto(leaderFsm, LS_WAIT_JOINER_IN_POSITION);
//            }
//        }
//        break;
//    }
//    case FSM_Exit(LS_WAIT_JOINER_IN_POSITION): {
//
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//            //the joiner is now in position and is ready to join
//            if (maneuver->getMessageType() == JM_IN_POSITION) {
//
//                //tell him to join the platoon
//                toSend = generateMessage();
//                toSend->setMessageType(LM_JOIN_PLATOON);
//                sendUnicast(toSend, vehicleData.joinerId);
//
//                FSM_Goto(leaderFsm, LS_WAIT_JOINER_TO_JOIN);
//
//            }
//        }
//
//        break;
//    }
//    case FSM_Exit(LS_WAIT_JOINER_TO_JOIN): {
//
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//            //the joiner has joined the platoon
//            if (maneuver->getMessageType() == JM_IN_PLATOON) {
//                //add the joiner to the list of vehicles in the platoon
//                vehicleData.formation.push_back(vehicleData.joinerId);
//                toSend = generateMessage();
//                toSend->setMessageType(LM_UPDATE_FORMATION);
//                toSend->setPlatoonFormationArraySize(
//                        vehicleData.formation.size());
//                for (unsigned int i = 0; i < vehicleData.formation.size();
//                        i++) {
//                    toSend->setPlatoonFormation(i, vehicleData.formation[i]);
//                }
//                //send to all vehicles
//                sendUnicast(toSend, -1);
//
//                FSM_Goto(leaderFsm, LS_LEADING);
//
//            }
//
//        }
//
//        break;
//    }
//
//    }
//
//    if (encapsulated) {
//        delete encapsulated;
//    }
//    if (unicast) {
//        delete unicast;
//    }
//
//}
//
//void BikesApp::handleJoinerMsg(cMessage *msg) {
//
//    //this message can be a self message, or a unicast message
//    //with an encapsulated beacon or maneuver message
//    PlatooningBeacon *beacon = 0;
//    ManeuverMessage *maneuver = 0;
//    cPacket *encapsulated = 0;
//    //maneuver message to be sent, if needed
//    ManeuverMessage *toSend;
//
//    //first check if this is a unicast message, and in case if it is a beacon or a maneuver
//    UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
//    if (unicast) {
//        encapsulated = unicast->decapsulate();
//        beacon = dynamic_cast<PlatooningBeacon *>(encapsulated);
//        maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
//    }
//
//    //check current joiner status
//    FSM_Switch(joinerFsm)
//    {
//
//    //init state, just move to the idle state
//    case FSM_Exit(JS_INIT): {
//        FSM_Goto(joinerFsm, JS_IDLE);
//        break;
//    }
//
//    case FSM_Exit(JS_IDLE): {
//        //if this is a self message triggering the beginning of procedure, then ask for joining
//        if (msg == startManeuver) {
//            toSend = generateMessage();
//            toSend->setMessageType(JM_REQUEST_JOIN);
//            sendUnicast(toSend, vehicleData.leaderId);
//            FSM_Goto(joinerFsm, JS_WAIT_REPLY);
//        }
//        break;
//    }
//
//    case FSM_Exit(JS_WAIT_REPLY): {
//
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//
//            //if the leader told us to move in position, we can start approaching the platoon
//            if (maneuver->getMessageType() == LM_MOVE_IN_POSITION) {
//                //save some data about the platoon
//                vehicleData.frontId = maneuver->getFrontVehicleId();
//                vehicleData.joinLane = maneuver->getPlatoonLane();
//                leaderId = vehicleData.leaderId;
//                frontId = vehicleData.frontId;
//
//                //check for correct lane. if not in correct lane, change it
//                int currentLane = traciVehicle->getLaneIndex();
//                if (currentLane != vehicleData.joinLane) {
//                    traciVehicle->setFixedLane(vehicleData.joinLane);
//                }
//
//                //activate faked CACC. this way we can approach the front car using data obtained through GPS
//                traciVehicle->setCACCConstantSpacing(15);
//                //we have no data so far, so for the moment just initialize with some fake data
//                traciVehicle->setControllerFakeData(15, vehicleData.speed, 0,
//                        vehicleData.speed, 0);
//                //set a CC speed higher than the platoon speed to approach it
//                traciVehicle->setCruiseControlDesiredSpeed(
//                        vehicleData.speed + 30 / 3.6);
//                traciVehicle->setActiveController(Plexe::FAKED_CACC);
//                FSM_Goto(joinerFsm, JS_MOVE_IN_POSITION);
//            }
//
//        }
//        break;
//    }
//
//    case FSM_Exit(JS_MOVE_IN_POSITION): {
//
//        //if we get data, just feed the fake CACC
//        if (beacon) {
//            if (beacon->getVehicleId() == vehicleData.leaderId) {
//                traciVehicle->setControllerFakeData(0, -1, 0,
//                        beacon->getSpeed(), beacon->getAcceleration());
//            }
//            if (beacon->getVehicleId() == vehicleData.frontId) {
//                //get front vehicle position
//                Coord frontPosition(beacon->getPositionX(),
//                        beacon->getPositionY(), 0);
//                //get my position
//                Coord position = mobility->getCurrentPosition();
//                //compute distance (-4 because of vehicle length)
//                double distance = position.distance(frontPosition) - 4;
//                traciVehicle->setControllerFakeData(distance,
//                        beacon->getSpeed(), beacon->getAcceleration(), -1, 0);
//                //if we are in position, tell the leader about that
//                if (distance < 16) {
//                    toSend = generateMessage();
//                    toSend->setMessageType(JM_IN_POSITION);
//                    sendUnicast(toSend, vehicleData.leaderId);
//                    FSM_Goto(joinerFsm, JS_WAIT_JOIN);
//                }
//            }
//        }
//        break;
//    }
//
//    case FSM_Exit(JS_WAIT_JOIN): {
//
//        //if while waiting approval from the leader we still get data packets, feed the CACC
//        if (beacon) {
//            if (beacon->getVehicleId() == vehicleData.leaderId) {
//                traciVehicle->setControllerFakeData(0, -1, 0,
//                        beacon->getSpeed(), beacon->getAcceleration());
//            }
//            if (beacon->getVehicleId() == vehicleData.frontId) {
//                //get front vehicle position
//                Coord frontPosition(beacon->getPositionX(),
//                        beacon->getPositionY(), 0);
//                //get my position
//                Coord position = mobility->getCurrentPosition();
//                //compute distance
//                double distance = position.distance(frontPosition) - 4;
//                traciVehicle->setControllerFakeData(distance,
//                        beacon->getSpeed(), beacon->getAcceleration(), -1, 0);
//            }
//        }
//
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//
//            //if we get confirmation from the leader, switch from faked CACC to real CACC
//            if (maneuver->getMessageType() == LM_JOIN_PLATOON) {
//                traciVehicle->setActiveController(Plexe::CACC);
//                //set spacing to 5 meters to get close to the platoon
//                traciVehicle->setCACCConstantSpacing(5);
//            }
//            //tell the leader that we're now in the platoon
//            toSend = generateMessage();
//            toSend->setMessageType(JM_IN_PLATOON);
//            sendUnicast(toSend, vehicleData.leaderId);
//            FSM_Goto(joinerFsm, JS_FOLLOW);
//
//        }
//
//        break;
//
//    }
//
//    case FSM_Exit(JS_FOLLOW): {
//
//        //we're now following. if we get an update of the formation, change it accordingly
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//            if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
//                vehicleData.formation.clear();
//                for (unsigned int i = 0;
//                        i < maneuver->getPlatoonFormationArraySize(); i++) {
//                    vehicleData.formation.push_back(
//                            maneuver->getPlatoonFormation(i));
//                }
//            }
//        }
//
//        break;
//    }
//
//    }
//
//    if (encapsulated) {
//        delete encapsulated;
//    }
//    if (unicast) {
//        delete unicast;
//    }
//
//}
//
//void BikesApp::handleFollowerMsg(cMessage *msg) {
//
//    //this message can be a self message, or a unicast message
//    //with an encapsulated beacon or maneuver message
//    ManeuverMessage *maneuver = 0;
//    cPacket *encapsulated = 0;
//
//    //first check if this is a unicast message, and in case if it is a beacon or a maneuver
//    UnicastMessage *unicast = dynamic_cast<UnicastMessage *>(msg);
//    if (unicast) {
//        encapsulated = unicast->decapsulate();
//        maneuver = dynamic_cast<ManeuverMessage *>(encapsulated);
//    }
//
//    //check current follower status
//    FSM_Switch(followerFsm)
//    {
//
//    case FSM_Exit(FS_INIT): {
//        FSM_Goto(followerFsm, FS_FOLLOW);
//        break;
//    }
//
//    case FSM_Exit(FS_FOLLOW): {
//
//        if (maneuver && maneuver->getPlatoonId() == vehicleData.platoonId) {
//            if (maneuver->getMessageType() == LM_UPDATE_FORMATION) {
//                vehicleData.formation.clear();
//                for (unsigned int i = 0;
//                        i < maneuver->getPlatoonFormationArraySize(); i++) {
//                    vehicleData.formation.push_back(
//                            maneuver->getPlatoonFormation(i));
//                }
//            }
//        }
//
//        break;
//    }
//
//    }
//
//    if (encapsulated) {
//        delete encapsulated;
//    }
//    if (unicast) {
//        delete unicast;
//    }
//
//}

void BikesApp::handleLowerControl( cMessage *msg )
{
	//lower control message
	UnicastProtocolControlMessage *ctrl = 0;

	ctrl = dynamic_cast<UnicastProtocolControlMessage *>( msg );
	//TODO: check for double free corruption
	if( ctrl )
	{
		delete ctrl;
	}
	else
	{
		delete msg;
	}
}

void BikesApp::onBeacon( WaveShortMessage* wsm )
{
}
